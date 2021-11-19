/*
 * TLS-Test Tool 
 * The TLS Test Tool checks the TLS configuration and compliance with the protocol specification for TLS servers and clients.
 * For more information visit https://www.achelos.de/de/tls-test-tool.html
 * 
 * Copyright (C) 2016 - 2021 achelos GmbH
 *
 * Licensed under the EUPL, Version 1.2 or – as soon they will be
 * approved by the European Commission - subsequent versions of the
 * EUPL (the "Licence");
 *
 * You may not use this work except in compliance with the Licence.
 * You may obtain a copy of the Licence at the LICENSE file or visit
 *
 * https://joinup.ec.europa.eu/collection/eupl
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the Licence is distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the Licence for the specific language governing permissions and
 * limitations under the Licence.
 */
#include "TlsLogFilter.h"
#include "logging/Logger.h"
#include "mbedtls/ecdh.h"
#include "strings/HexStringHelper.h"
#include "strings/StringHelper.h"
#include "tls/TlsHandshakeHeader.h"
#include "tls/TlsPlaintextHeader.h"
#include "tls/TlsHeartbeatMessageHeader.h"
#include <cstdio>
#include <functional>
#include <regex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace TlsTestTool {
namespace MbedTls {

static std::string mpiToHexString(const mbedtls_mpi & mpi) {
	std::size_t requiredSize;
	mbedtls_mpi_write_string(&mpi, 16, nullptr, 0, &requiredSize);
	std::vector<char> buffer(requiredSize + 1);
	mbedtls_mpi_write_string(&mpi, 16, buffer.data(), buffer.size(), &requiredSize);
	// Add spaces for separating the bytes
	std::stringstream stream;
	std::size_t numWritten = 0;
	for (auto digit : buffer) {
		if (0 != std::isxdigit(digit)) {
			stream << static_cast<char>(std::tolower(digit));
			++numWritten;
		}
		if (2 == numWritten) {
			stream << ' ';
			numWritten = 0;
		}
	}
	return stream.str();
}

static void logHandshakeMessage(Tooling::Logger & logger, const TlsVersion & version, const uint8_t * data,
								const std::size_t size) {
	if (sizeof(TlsHandshakeHeader) > size) {
		return;
	}
	const auto handshakeHeader = reinterpret_cast<const TlsHandshakeHeader *>(data);
	data += sizeof(TlsHandshakeHeader);
	if (TlsHandshakeType::CERTIFICATE == handshakeHeader->msgType) {
		if ((sizeof(TlsHandshakeHeader) + handshakeHeader->length.get()) > size) {
			return;
		}
		const auto certificateListLength = reinterpret_cast<const TlsUint24 *>(data)->get();
		data += sizeof(TlsUint24);
		uint32_t numCertificate = 0;
		for (std::size_t certificateDataRead = 0; certificateDataRead < certificateListLength;) {
			const auto certificateLength = reinterpret_cast<const TlsUint24 *>(data)->get();
			data += sizeof(TlsUint24);
			logger.log(Tooling::LogLevel::HIGH, "TLS", __FILE__, __LINE__, "Certificate.certificate_list["
							   + std::to_string(numCertificate) + "]="
							   + Tooling::HexStringHelper::byteArrayToHexString({data, data + certificateLength}));
			data += certificateLength;
			certificateDataRead += sizeof(TlsUint24) + certificateLength;
			++numCertificate;
		}
		logger.log(Tooling::LogLevel::HIGH, "TLS", __FILE__, __LINE__,
				   "Certificate.certificate_list.size=" + std::to_string(numCertificate));
	} else if (TlsHandshakeType::CERTIFICATE_REQUEST == handshakeHeader->msgType) {
		if ((sizeof(TlsHandshakeHeader) + handshakeHeader->length.get()) > size) {
			return;
		}
		const auto certificateTypesLength = *data;
		data += sizeof(TlsUint8);
		logger.log(Tooling::LogLevel::HIGH, "TLS", __FILE__, __LINE__, "CertificateRequest.certificate_types="
						   + Tooling::HexStringHelper::byteArrayToHexString({data, data + certificateTypesLength}));
		data += certificateTypesLength;
		if (TLS_VERSION_TLS_1_2 == version) {
			// supported_signature_algorithms is only contained in TLS 1.2
			const auto supportedSignatureAlgorithmsLength = reinterpret_cast<const TlsUint16 *>(data)->get();
			data += sizeof(TlsUint16);
			logger.log(Tooling::LogLevel::HIGH, "TLS", __FILE__, __LINE__,
					   "CertificateRequest.supported_signature_algorithms="
							   + Tooling::HexStringHelper::byteArrayToHexString(
										 {data, data + supportedSignatureAlgorithmsLength}));
			data += supportedSignatureAlgorithmsLength;
		}
		const auto certificateAuthoritiesLength = reinterpret_cast<const TlsUint16 *>(data)->get();
		data += sizeof(TlsUint16);
		logger.log(
				Tooling::LogLevel::HIGH, "TLS", __FILE__, __LINE__, "CertificateRequest.certificate_authorities="
						+ Tooling::HexStringHelper::byteArrayToHexString({data, data + certificateAuthoritiesLength}));
		data += certificateAuthoritiesLength;
	}
}

static void logTlsRecord(Tooling::Logger & logger, const uint8_t * data, const std::size_t size) {
	if (sizeof(TlsPlaintextHeader) > size) {
		return;
	}
	const auto plaintextHeader = reinterpret_cast<const TlsPlaintextHeader *>(data);
	if (TlsContentType::HANDSHAKE == plaintextHeader->type) {
		logHandshakeMessage(logger, plaintextHeader->version, data + sizeof(TlsPlaintextHeader),
							size - sizeof(TlsPlaintextHeader));
	}
}

static void logHeartbeatMessage(Tooling::Logger & logger, const uint8_t * data, const std::size_t size) {
    if (sizeof(HeartbeatMessageHeader) > size) {
        return;
    }
    auto const * heartbeatMessageHeader = reinterpret_cast<const HeartbeatMessageHeader *>(data);
    const uint8_t * payloadBegin = data + sizeof(HeartbeatMessageHeader);
    const uint8_t * payloadEnd = data + sizeof(HeartbeatMessageHeader) + heartbeatMessageHeader->payload_length.get();
    logger.log(Tooling::LogLevel::HIGH, "TLS", __FILE__, __LINE__, "Heartbeat data size including padding=" + std::to_string(size));
    logger.log(Tooling::LogLevel::HIGH, "TLS", __FILE__, __LINE__, "Heartbeat.type=" + std::to_string(heartbeatMessageHeader->type));
    logger.log(Tooling::LogLevel::HIGH, "TLS", __FILE__, __LINE__, "Heartbeat.payload_length=" + std::to_string(heartbeatMessageHeader->payload_length.get()));
    logger.log(Tooling::LogLevel::HIGH, "TLS", __FILE__, __LINE__,
               "Heartbeat.payload_data=" + Tooling::HexStringHelper::byteArrayToHexString({payloadBegin, payloadEnd}));

}

static void logHeartbeatRecord(Tooling::Logger & logger, const uint8_t * data, const std::size_t size) {
    if (sizeof(TlsPlaintextHeader) > size) {
        return;
    }
    const auto plaintextHeader = reinterpret_cast<const TlsPlaintextHeader *>(data);
    if (TlsContentType::HEARTBEAT == plaintextHeader->type) {
        logHeartbeatMessage(logger, data + sizeof(TlsPlaintextHeader), size - sizeof(TlsPlaintextHeader));
    }
}

static const std::unordered_map<std::string, std::string> logStringTranslation{
		// Receiving
		{"<= parse client hello\n", "Valid ClientHello message received.\n"},
		{"bad client hello message\n", "Bad ClientHello message received.\n"},
		{"<= parse server hello\n", "Valid ServerHello message received.\n"},
		{"bad server hello message\n", "Bad ServerHello message received.\n"},
		{"<= parse certificate\n", "Valid Certificate message received.\n"},
		{"bad certificate message\n", "Bad Certificate message received.\n"},
		{"<= parse server key exchange\n", "Valid ServerKeyExchange message received.\n"},
		{"bad server key exchange message\n", "Bad ServerKeyExchange message received.\n"},
		{"got a certificate request\n", "Valid CertificateRequest message received.\n"},
		{"bad certificate request message\n", "Bad CertificateRequest message received.\n"},
		{"<= parse server hello done\n", "Valid ServerHelloDone message received.\n"},
		{"bad server hello done message\n", "Bad ServerHelloDone message received.\n"},
		{"<= parse client key exchange\n", "Valid ClientKeyExchange message received.\n"},
		{"bad client key exchange\n", "Bad ClientKeyExchange message received.\n"},
		{"<= parse certificate verify\n", "Valid CertificateVerify message received.\n"},
		{"bad certificate verify message\n", "Bad CertificateVerify message received.\n"},
		{"<= parse change cipher spec\n", "Valid ChangeCipherSpec message received.\n"},
		{"bad change cipher spec message\n", "Bad ChangeCipherSpec message received.\n"},
		{"<= parse finished\n", "Valid Finished message received.\n"},
		{"bad finished message\n", "Bad Finished message received.\n"},
		// Transmitting
		{"<= write client hello\n", "ClientHello message transmitted.\n"},
		{"<= write server hello\n", "ServerHello message transmitted.\n"},
		{"<= write certificate\n", "Certificate message transmitted.\n"},
		{"<= write server key exchange\n", "ServerKeyExchange message transmitted.\n"},
		{"<= write certificate request\n", "CertificateRequest message transmitted.\n"},
		{"<= write server hello done\n", "ServerHelloDone message transmitted.\n"},
		{"<= write client key exchange\n", "ClientKeyExchange message transmitted.\n"},
		{"<= write certificate verify\n", "CertificateVerify message transmitted.\n"},
		{"<= write change cipher spec\n", "ChangeCipherSpec message transmitted.\n"},
		{"<= write finished\n", "Finished message transmitted.\n"},
};
static void translateString(Tooling::Logger & logger, const std::string & message) {
	auto stringTranslation = logStringTranslation.find(message);
	if (logStringTranslation.cend() != stringTranslation) {
		logger.log(Tooling::LogLevel::HIGH, "TLS", __FILE__, __LINE__, stringTranslation->second);
	}
}

static const std::vector<std::pair<std::regex, std::string>> logRegExTranslation{
		{std::regex{"server hello, received ciphersuite: ([0-9a-f]{2})([0-9a-f]{2})\n"},
		 "ServerHello.cipher_suite=$1 $2\n"},
		{std::regex{"server hello, chosen ciphersuite: ([0-9a-f]{2})([0-9a-f]{2})\n"},
		 "ServerHello.cipher_suite=$1 $2\n"},
		{std::regex{"got an alert message, type: \\[[0-9]+:[0-9]+\\]\n"}, "Alert message received.\n"},
		{std::regex{"padding_length: ([0-9a-f]{2})\n"}, "Finished.GenericBlockCipher.padding_length=$1\n"},
};
static void translateRegEx(Tooling::Logger & logger, const std::string & message) {
	for (const auto & regExTranslation : logRegExTranslation) {
		std::smatch match;
		if (std::regex_match(message, match, regExTranslation.first)) {
			logger.log(Tooling::LogLevel::HIGH, "TLS", __FILE__, __LINE__, match.format(regExTranslation.second));
		}
	}
}

static const std::vector<std::pair<std::regex, std::string>> logRegExDecToHexTranslation{
		{std::regex{"server hello, compress alg.: ([0-9]+)\n"}, "ServerHello.compression_method=%02x\n"},
		{std::regex{"got an alert message, type: \\[([0-9]+):[0-9]+\\]\n"}, "Alert.level=%02x\n"},
		{std::regex{"got an alert message, type: \\[[0-9]+:([0-9]+)\\]\n"}, "Alert.description=%02x\n"},
		{std::regex{"Server used HashAlgorithm ([0-9]+)\n"}, "ServerKeyExchange.signed_params.algorithm.hash=%02x\n"},
		{std::regex{"Server used SignatureAlgorithm ([0-9]+)\n"},
		 "ServerKeyExchange.signed_params.algorithm.signature=%02x\n"},
};
static void translateRegExDecToHex(Tooling::Logger & logger, const std::string & message) {
	for (const auto & regExTranslation : logRegExDecToHexTranslation) {
		std::smatch match;
		if (std::regex_match(message, match, regExTranslation.first)) {
			auto number = std::stoul(match[1]);
			auto formatString = regExTranslation.second;
			logger.log(Tooling::LogLevel::HIGH, "TLS", __FILE__, __LINE__,
					   Tooling::StringHelper::formatInt(formatString, number));
		}
	}
}

static const std::unordered_map<std::string, std::string> dumpInterception{
		{"client hello, version", "ClientHello.client_version"},
		{"client hello, random bytes", "ClientHello.random"},
		{"client hello, session id", "ClientHello.session_id"},
		{"client hello, ciphersuitelist", "ClientHello.cipher_suites"},
		{"client hello, compression", "ClientHello.compression_methods"},
		{"client hello extensions", "ClientHello.extensions"},
		{"server hello, version", "ServerHello.server_version"},
		{"server hello, random bytes", "ServerHello.random"},
		{"server hello, session id", "ServerHello.session_id"},
		{"server hello, extensions", "ServerHello.extensions"},
		{"server key exchange", "ServerKeyExchange"},
		{"signature", "ServerKeyExchange.signed_params.signature"},
		{"md5_hash", "ServerKeyExchange.signed_params.md5_hash"},
		{"sha_hash", "ServerKeyExchange.signed_params.sha_hash"},
		{"premaster secret", "ClientKeyExchange.exchange_keys.pre_master_secret"},
		{"master secret", "ClientKeyExchange.exchange_keys.master_secret"},
		{"input record from network", "TLS Record"},
		{"remaining content in record", "Handshake Message"},
		{"new session ticket, ticket", "NewSessionTicket.ticket"},
        {"heartbeat input record after decrypt", "Heartbeat Record"},
};
/*
 * Intercept lines like
 *
 * dumping 'server hello, version' (2 bytes)
 * 0000:  03 03
 *
 * or
 *
 * dumping 'server hello, random bytes' (32 bytes)
 * 0000:  16 52 57 ea af a6 f5 3f b9 2b e0 34 da e2 c3 e7  .RW....?.+.4....
 * 0010:  6b 80 cf 3f 1e b4 74 04 d7 68 49 2a dd 6a a4 76  k..?..t..hI*.j.v
 *
 * There are at most 16 bytes per line.
 */
static void interceptHexDump(Tooling::Logger & logger, const std::string & message) {
	static const std::regex dumpingHeaderRegEx{"dumping '([^']+)' \\(([0-9]+) bytes\\)\n"};
	static const std::regex hexDumpRegEx{"[0-9a-f]{4}:  (([0-9a-f]{2} ){1,16}) .*\n"};
	static long numLinesToCollect{0};
	static std::string collectedBytes{};
	static std::string finalOutput{};
	// Check, if a string in the dumping header is defined to be intercepted
	std::smatch dumpingHeaderMatch;
	if (std::regex_match(message, dumpingHeaderMatch, dumpingHeaderRegEx)) {
		auto interception = dumpInterception.find(dumpingHeaderMatch[1]);
		if (dumpInterception.cend() != interception) {
			// Instruct to intercept the next lines
			const auto numBytes = std::stoul(dumpingHeaderMatch[2]);
			numLinesToCollect = (numBytes + 15) / 16;
			collectedBytes.clear();
			finalOutput = interception->second;
		}
	}
	std::smatch hexDumpMatch;
	if ((0 < numLinesToCollect) && std::regex_match(message, hexDumpMatch, hexDumpRegEx)) {
		// Append the current hex dump line
		collectedBytes += hexDumpMatch[1];
		--numLinesToCollect;
	}
	if ((0 == numLinesToCollect) && !finalOutput.empty()) {
		if (finalOutput == "ServerKeyExchange") {
			auto byteArray = Tooling::HexStringHelper::hexStringToByteArray(collectedBytes);
			mbedtls_ecdh_context ecdhContext;
			mbedtls_ecdh_init(&ecdhContext);
			const unsigned char * arrayStart = byteArray.data();
			mbedtls_ecdh_read_params(&ecdhContext, &arrayStart, arrayStart + byteArray.size());
			auto curveInfo = mbedtls_ecp_curve_info_from_grp_id(ecdhContext.grp.id);
			if (nullptr != curveInfo) {
				logger.log(Tooling::LogLevel::HIGH, "TLS", __FILE__, __LINE__,
						   Tooling::StringHelper::formatInt("ServerKeyExchange.params.curve_params.namedcurve=%02x",
															curveInfo->tls_id));
				logger.log(Tooling::LogLevel::HIGH, "TLS", __FILE__, __LINE__,
						   std::string{"ServerKeyExchange.params.public="} + "04 " + mpiToHexString(ecdhContext.Qp.X)
								   + mpiToHexString(ecdhContext.Qp.Y));
			}
			mbedtls_ecdh_free(&ecdhContext);
		} else if (finalOutput == "TLS Record") {
			const auto byteArray = Tooling::HexStringHelper::hexStringToByteArray(collectedBytes);
			logTlsRecord(logger, byteArray.data(), byteArray.size());
		} else if (finalOutput == "Handshake Message") {
			const auto byteArray = Tooling::HexStringHelper::hexStringToByteArray(collectedBytes);
			logHandshakeMessage(logger, TLS_VERSION_INVALID, byteArray.data(), byteArray.size());
        } else if (finalOutput == "Heartbeat Record") {
            const auto byteArray = Tooling::HexStringHelper::hexStringToByteArray(collectedBytes);
            logHeartbeatRecord(logger, byteArray.data(), byteArray.size());
        } else {
			logger.log(Tooling::LogLevel::HIGH, "TLS", __FILE__, __LINE__, finalOutput + '=' + collectedBytes);
		}
		finalOutput.clear();
		collectedBytes.clear();
	}
}

static const std::unordered_map<std::string, std::string> bitsInterception{
		{"DHM: P ", "ServerKeyExchange.params.dh_p"},
		{"DHM: G ", "ServerKeyExchange.params.dh_g"},
		{"DHM: GY", "ServerKeyExchange.params.dh_Ys"},
};
/*
 * Intercept lines like
 *
 * value of 'DHM: G ' (2 bits) is:
 *  02
 *
 * or
 *
 * value of 'DHM: GY' (4096 bits) is:
 *  dd 6d 24 9d 25 29 ab 44 83 b4 9c 31 21 b0 86 0e
 *  a6 c9 99 84 e0 fb 72 6e 29 fe 1f d3 24 36 b9 97
 *  e9 c6 d6 9d b2 8c c8 a0 ff 73 f3 b3 03 69 53 67
 * ...
 *
 * There are at most 16 bytes per line.
 */
static void interceptBits(Tooling::Logger & logger, const std::string & message) {
	static const std::regex bitsHeaderRegEx{"value of '([^']+)' \\(([0-9]+) bits\\) is:\n"};
	static const std::regex bitsRegEx{" (([0-9a-f]{2} ?){1,16})\n"};
	static std::string finalOutput{};
	if (!finalOutput.empty()) {
		std::smatch bitsMatch;
		if (std::regex_match(message, bitsMatch, bitsRegEx)) {
			// Append the current bits line
			finalOutput += bitsMatch[1];
			finalOutput += ' ';
		} else {
			logger.log(Tooling::LogLevel::HIGH, "TLS", __FILE__, __LINE__, finalOutput);
			finalOutput.clear();
		}
	}
	// Check, if a string in the bits header is defined to be intercepted
	std::smatch bitsHeaderMatch;
	if (finalOutput.empty() && std::regex_match(message, bitsHeaderMatch, bitsHeaderRegEx)) {
		auto interception = bitsInterception.find(bitsHeaderMatch[1]);
		if (bitsInterception.cend() != interception) {
			// Instruct to intercept the next lines
			finalOutput = interception->second + '=';
		}
	}
}

void TlsLogFilter::registerInstances(Tooling::Logger & logger) {
	using namespace std::placeholders;
	logger.addLogFilter(std::bind(&translateString, _1, _4));
	logger.addLogFilter(std::bind(&translateRegEx, _1, _4));
	logger.addLogFilter(std::bind(&translateRegExDecToHex, _1, _4));
	logger.addLogFilter(std::bind(&interceptHexDump, _1, _4));
	logger.addLogFilter(std::bind(&interceptBits, _1, _4));
}
}
}
