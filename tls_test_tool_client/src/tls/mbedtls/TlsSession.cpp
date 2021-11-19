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
#include "TlsSession.h"
#include "TlsLogFilter.h"
#include "logging/Logger.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/debug.h"
#include "mbedtls/entropy.h"
#include "mbedtls/error.h"
#include "mbedtls/pk.h"
#include "mbedtls/pk_internal.h"
#include "mbedtls/ssl.h"
#include "mbedtls/ssl_internal.h"
#include "mbedtls/version.h"
#include "network/TcpClient.h"
#include "network/TcpServer.h"
#include "strings/HexStringHelper.h"
#include "tls/TlsMessage.h"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <thread>

namespace TlsTestTool {
namespace MbedTls {

static Tooling::LogLevel convertLogLevel(int level) {
	if (3 < level) {
		return Tooling::LogLevel::LOW;
	} else if (2 < level) {
		return Tooling::LogLevel::MEDIUM;
	} else if (0 < level) {
		return Tooling::LogLevel::HIGH;
	} else {
		return Tooling::LogLevel::OFF;
	}
}

static TlsHandshakeState convertState(const int mbedTlsState) {
	switch (mbedTlsState) {
		case MBEDTLS_SSL_HELLO_REQUEST:
			return TlsHandshakeState::HELLO_REQUEST;
		case MBEDTLS_SSL_CLIENT_HELLO:
			return TlsHandshakeState::CLIENT_HELLO;
		case MBEDTLS_SSL_SERVER_HELLO:
			return TlsHandshakeState::SERVER_HELLO;
		case MBEDTLS_SSL_SERVER_CERTIFICATE:
			return TlsHandshakeState::SERVER_CERTIFICATE;
		case MBEDTLS_SSL_SERVER_KEY_EXCHANGE:
			return TlsHandshakeState::SERVER_KEY_EXCHANGE;
		case MBEDTLS_SSL_CERTIFICATE_REQUEST:
			return TlsHandshakeState::CERTIFICATE_REQUEST;
		case MBEDTLS_SSL_SERVER_HELLO_DONE:
			return TlsHandshakeState::SERVER_HELLO_DONE;
		case MBEDTLS_SSL_CLIENT_CERTIFICATE:
			return TlsHandshakeState::CLIENT_CERTIFICATE;
		case MBEDTLS_SSL_CLIENT_KEY_EXCHANGE:
			return TlsHandshakeState::CLIENT_KEY_EXCHANGE;
		case MBEDTLS_SSL_CERTIFICATE_VERIFY:
			return TlsHandshakeState::CERTIFICATE_VERIFY;
		case MBEDTLS_SSL_CLIENT_CHANGE_CIPHER_SPEC:
			return TlsHandshakeState::CLIENT_CHANGE_CIPHER_SPEC;
		case MBEDTLS_SSL_CLIENT_FINISHED:
			return TlsHandshakeState::CLIENT_FINISHED;
		case MBEDTLS_SSL_SERVER_CHANGE_CIPHER_SPEC:
			return TlsHandshakeState::SERVER_CHANGE_CIPHER_SPEC;
		case MBEDTLS_SSL_SERVER_FINISHED:
			return TlsHandshakeState::SERVER_FINISHED;
		case MBEDTLS_SSL_FLUSH_BUFFERS:
			return TlsHandshakeState::INTERNAL_1;
		case MBEDTLS_SSL_HANDSHAKE_WRAPUP:
			return TlsHandshakeState::INTERNAL_2;
		case MBEDTLS_SSL_HANDSHAKE_OVER:
			return TlsHandshakeState::HANDSHAKE_DONE;
		case MBEDTLS_SSL_SERVER_NEW_SESSION_TICKET:
		case MBEDTLS_SSL_SERVER_HELLO_VERIFY_REQUEST_SENT:
		default:
			throw std::invalid_argument{"Unsupported TLS handshake state"};
	}
}

static int convertState(const TlsHandshakeState tlsState) {
	switch (tlsState) {
		case TlsHandshakeState::HELLO_REQUEST:
			return MBEDTLS_SSL_HELLO_REQUEST;
		case TlsHandshakeState::CLIENT_HELLO:
			return MBEDTLS_SSL_CLIENT_HELLO;
		case TlsHandshakeState::SERVER_HELLO:
			return MBEDTLS_SSL_SERVER_HELLO;
		case TlsHandshakeState::SERVER_CERTIFICATE:
			return MBEDTLS_SSL_SERVER_CERTIFICATE;
		case TlsHandshakeState::SERVER_KEY_EXCHANGE:
			return MBEDTLS_SSL_SERVER_KEY_EXCHANGE;
		case TlsHandshakeState::CERTIFICATE_REQUEST:
			return MBEDTLS_SSL_CERTIFICATE_REQUEST;
		case TlsHandshakeState::SERVER_HELLO_DONE:
			return MBEDTLS_SSL_SERVER_HELLO_DONE;
		case TlsHandshakeState::CLIENT_CERTIFICATE:
			return MBEDTLS_SSL_CLIENT_CERTIFICATE;
		case TlsHandshakeState::CLIENT_KEY_EXCHANGE:
			return MBEDTLS_SSL_CLIENT_KEY_EXCHANGE;
		case TlsHandshakeState::CERTIFICATE_VERIFY:
			return MBEDTLS_SSL_CERTIFICATE_VERIFY;
		case TlsHandshakeState::CLIENT_CHANGE_CIPHER_SPEC:
			return MBEDTLS_SSL_CLIENT_CHANGE_CIPHER_SPEC;
		case TlsHandshakeState::CLIENT_FINISHED:
			return MBEDTLS_SSL_CLIENT_FINISHED;
		case TlsHandshakeState::SERVER_CHANGE_CIPHER_SPEC:
			return MBEDTLS_SSL_SERVER_CHANGE_CIPHER_SPEC;
		case TlsHandshakeState::SERVER_FINISHED:
			return MBEDTLS_SSL_SERVER_FINISHED;
		case TlsHandshakeState::INTERNAL_1:
			return MBEDTLS_SSL_FLUSH_BUFFERS;
		case TlsHandshakeState::INTERNAL_2:
			return MBEDTLS_SSL_HANDSHAKE_WRAPUP;
		case TlsHandshakeState::HANDSHAKE_DONE:
			return MBEDTLS_SSL_HANDSHAKE_OVER;
		default:
			throw std::invalid_argument{"Unsupported TLS handshake state"};
	}
}

static std::string errorToString(const int errorCode) {
	std::array<char, 1024> str;
	mbedtls_strerror(errorCode, str.data(), str.size());
	return {str.data(), str.size()};
}

static void assertSuccess(const std::string & functionName, const int result) {
	if (0 != result) {
		throw std::runtime_error(functionName + " failed: " + errorToString(result));
	}
}

static void writeToLogger(void * context, int level, const char * file, int line, const char * str) {
	auto logger = reinterpret_cast<Tooling::Logger *>(context);
	logger->log(convertLogLevel(level), "mbedTLS", file, line, str);
}

static void parseCertificate(std::istream & certificateInput, mbedtls_x509_crt * certificateOutput) {
	std::stringstream buffer;
	buffer << certificateInput.rdbuf() << '\0';
	const std::string data{buffer.str()};
	const auto result = mbedtls_x509_crt_parse(certificateOutput, reinterpret_cast<const unsigned char *>(data.c_str()),
											   data.size());
	assertSuccess("mbedtls_x509_crt_parse", result);
}

static void parsePrivateKey(std::istream & privateKeyInput, mbedtls_pk_context * privateKeyOutput) {
	std::stringstream buffer;
	buffer << privateKeyInput.rdbuf() << '\0';
	const std::string data{buffer.str()};
	const auto result = mbedtls_pk_parse_key(privateKeyOutput, reinterpret_cast<const unsigned char *>(data.c_str()),
											 data.size(), nullptr, 0);
	assertSuccess("mbedtls_pk_parse_key", result);
}

class TlsSession::Data {
public:
	TlsSession & tlsSession;
	std::vector<int> tlsCipherSuites;
	std::vector<uint8_t> clientHelloCompressionMethods;
	std::vector<uint8_t> clientHelloExtensions;
	uint64_t sequenceNumber;
	std::vector<uint8_t> clientRandom;
	int oldSecureRenegotiation;
	bool expectAlertMessage;
	bool renegotiationStarted;
	uint32_t waitForAlertSeconds;
	uint32_t tcpReceiveTimeoutSeconds;
	mbedtls_ctr_drbg_context ctrDrbg;
	mbedtls_entropy_context entropy;
	mbedtls_ssl_config conf;
	mbedtls_ssl_context ssl;

	mbedtls_x509_crt certificate;
	mbedtls_pk_context privateKey;

	Data(TlsSession & newTlsSession)
			: tlsSession(newTlsSession),
			  tlsCipherSuites(),
			  clientHelloCompressionMethods(),
			  clientHelloExtensions(),
			  sequenceNumber(0),
			  clientRandom(),
			  oldSecureRenegotiation(0),
			  expectAlertMessage(false),
			  renegotiationStarted(false),
			  waitForAlertSeconds(10000),
			  tcpReceiveTimeoutSeconds(120000) {
		mbedtls_ctr_drbg_init(&ctrDrbg);
		mbedtls_entropy_init(&entropy);
		mbedtls_ssl_init(&ssl);
		mbedtls_ssl_config_init(&conf);

		const std::string pers{"tls_test_tool"};
		assertSuccess("mbedtls_ctr_drbg_seed",
					  mbedtls_ctr_drbg_seed(&ctrDrbg, mbedtls_entropy_func, &entropy,
											reinterpret_cast<const unsigned char *>(pers.c_str()), pers.length()));

		assertSuccess("mbedtls_ssl_config_defaults",
					  mbedtls_ssl_config_defaults(&conf,
												  tlsSession.isClient() ? MBEDTLS_SSL_IS_CLIENT : MBEDTLS_SSL_IS_SERVER,
												  MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT));

		mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctrDrbg);
		mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_NONE);

		assertSuccess("mbedtls_ssl_setup", mbedtls_ssl_setup(&ssl, &conf));

		mbedtls_ssl_set_bio(&ssl, this, tcpSend, tcpReceive, nullptr);

		mbedtls_x509_crt_init(&certificate);
		mbedtls_pk_init(&privateKey);
	}

	~Data() {
		mbedtls_pk_free(&privateKey);
		mbedtls_x509_crt_free(&certificate);

		mbedtls_ssl_config_free(&conf);
		mbedtls_ssl_free(&ssl);
		mbedtls_entropy_free(&entropy);
		mbedtls_ctr_drbg_free(&ctrDrbg);
	}

	bool isDataReadable() {
		return (0 < tlsSession.getSocket().available());
	}

	void tryToReadAlertMessage(bool forceRead) {
		// Prevent Alert from being ignored that are received before sending a ServerHello
		const auto oldMinorVersion = conf.max_minor_ver;
		conf.max_minor_ver = MBEDTLS_SSL_MINOR_VERSION_3;
		while (isDataReadable()) {
			tlsSession.log(__FILE__, __LINE__, "Checking for Alert message in received data.");
			const auto result = mbedtls_ssl_fetch_input(&ssl, 5);
			const bool msgHeaderIndicatesAlert = (0 == result) && (MBEDTLS_SSL_MSG_ALERT == ssl.in_hdr[0]);
			if (msgHeaderIndicatesAlert || forceRead) {
 				const auto result = mbedtls_ssl_read_record(&ssl,1);
				if (MBEDTLS_ERR_SSL_FATAL_ALERT_MESSAGE == result) {
					tlsSession.log(__FILE__, __LINE__, "Fatal Alert message received.");
					break;
				} else if (MBEDTLS_ERR_SSL_INVALID_RECORD == result) {
					tlsSession.log(__FILE__, __LINE__, "Invalid TLS record received.");
					tlsSession.log(__FILE__, __LINE__, "Stop searching for Alert message.");
					break;
				}
			} else if ((0 == result) && (MBEDTLS_SSL_MSG_APPLICATION_DATA == ssl.in_hdr[0])) {
				tlsSession.log(__FILE__, __LINE__, "Skipping application data in received data.");
				receiveApplicationData();
			} else {
				break;
			}
		}
		conf.max_minor_ver = oldMinorVersion;
	}

	void waitForExpectedAlertMessage(const bool msgWasSent) {
		if (!expectAlertMessage || !msgWasSent) {
			return;
		}
		tlsSession.log(__FILE__, __LINE__, "Waiting for incoming data that might contain an Alert message.");
		// Wait for at most ten seconds for data to arrive
		static const std::chrono::milliseconds timeout(waitForAlertSeconds);
		const auto timeStart = std::chrono::steady_clock::now();
		while (!isDataReadable()) {
			if (timeout < (std::chrono::steady_clock::now() - timeStart)) {
				break;
			}
			// Take small breaks to save resources.
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
		}
		tryToReadAlertMessage(false);
		expectAlertMessage = false;
	}

	std::vector<uint8_t> receiveApplicationData() {
		int read = 0;
		int block_size = 1024;
		int result;
		std::vector<uint8_t> buffer(static_cast<std::size_t>(block_size), static_cast<uint8_t>(0));

		while (true) {
			result = mbedtls_ssl_read(&ssl, buffer.data() + read, block_size);
			if ((MBEDTLS_ERR_SSL_WANT_READ == result) || (MBEDTLS_ERR_SSL_WANT_WRITE == result)) {
				continue;
			}

			if (MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY == result && 0 < read) {
				tlsSession.log(__FILE__, __LINE__, "Connection was closed gracefully.");
			} else if (MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY == result) {
				throw std::runtime_error("connection was closed gracefully.");
			} else if (0 == result && 0 < read) {
				tlsSession.log(__FILE__, __LINE__, "Connection was reset by peer.");
			} else if (0 == result) {
				throw std::runtime_error("connection was reset by peer.");
			} else if (0 > result) {
				throw std::runtime_error("mbedtls_ssl_read failed: " + errorToString(result));
			} else {
				read += result;
			}

			// check if more data is available
			if (block_size == result) {
				buffer.resize(read + block_size);
				continue;
			}

			buffer.resize(read);
			return buffer;
		}
	}

	static int tcpSend(void * context, const unsigned char * data, size_t size) {
		TlsSession::Data * dataContainer = reinterpret_cast<TlsSession::Data *>(context);
		if (dataContainer->renegotiationStarted && !dataContainer->tlsSession.isClient()) {
			dataContainer->tryToReadAlertMessage(false);
		}
		TcpClient & connection = dataContainer->tlsSession.getSocket();
		return connection.write({reinterpret_cast<const char *>(data), reinterpret_cast<const char *>(data) + size});
	}

	static int tcpReceive(void * context, unsigned char * data, size_t size) {
		TlsSession::Data * dataContainer = reinterpret_cast<TlsSession::Data *>(context);
		TcpClient & connection = dataContainer->tlsSession.getSocket();
		static const std::chrono::milliseconds receiveTimeout(dataContainer->tcpReceiveTimeoutSeconds);
		const auto timeStart = std::chrono::steady_clock::now();
		while (0 == connection.available()) {
			if (receiveTimeout < (std::chrono::steady_clock::now() - timeStart)) {
				return MBEDTLS_ERR_SSL_TIMEOUT;
			}
			/* Perform two checks here to circumvent a timing problem in isClosed where isReadable and nothingToRead are
			 * both true when data is incoming in multiple TCP fragments */
			if (dataContainer->tlsSession.isClient()) {
				if (connection.isClosed(true)) {
					return MBEDTLS_ERR_SSL_CONN_EOF;
				}
			} else {
				if (connection.isClosed() && connection.isClosed()) {
					return MBEDTLS_ERR_SSL_CONN_EOF;
				}
			}
			// Take small breaks to save resources.
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
		}
		const auto receivedData = connection.read(std::min(connection.available(), size));
		std::copy(receivedData.begin(), receivedData.end(), data);
		return receivedData.size();
	}
};

TlsSession::TlsSession(TcpClient & tcpClient)
		: TlsTestTool::TlsSession::TlsSession(tcpClient), impl(std::make_unique<Data>(*this)) {
}

TlsSession::TlsSession(TcpServer & tcpServer)
		: TlsTestTool::TlsSession::TlsSession(tcpServer), impl(std::make_unique<Data>(*this)) {
}

TlsSession::~TlsSession() = default;

void TlsSession::setCertificate(std::istream & certificate, std::istream & privateKey) {
	parseCertificate(certificate, &impl->certificate);
	parsePrivateKey(privateKey, &impl->privateKey);
	const auto result = mbedtls_ssl_conf_own_cert(&impl->conf, &impl->certificate, &impl->privateKey);
	assertSuccess("mbedtls_ssl_conf_own_cert", result);
}

void TlsSession::performHandshake() {
	try {
		while (TlsHandshakeState::HANDSHAKE_DONE != getState()) {
			performHandshakeStep();
			if (getSocket().isClosed(isClient())) {
				log(__FILE__, __LINE__, "Handshake aborted.");
				return;
			}
		}
	} catch (const std::exception &) {
		impl->expectAlertMessage = true;
		impl->waitForExpectedAlertMessage(true);
		throw;
	}
	log(__FILE__, __LINE__, "Handshake successful.");
	log(__FILE__, __LINE__, std::string{"Protocol: "} + mbedtls_ssl_get_version(&impl->ssl));
	log(__FILE__, __LINE__, std::string{"Cipher suite: "} + mbedtls_ssl_get_ciphersuite(&impl->ssl));
}

void TlsSession::performHandshakeStep() {
	onPreStep();
	const auto currentState = getState();
	bool expectingPeerFinishedMessage = false;
	if ((isClient() && (TlsHandshakeState::SERVER_FINISHED == currentState))
		|| (!isClient() && (TlsHandshakeState::CLIENT_FINISHED == currentState))) {
		expectingPeerFinishedMessage = true;
	}
	if (isSecrectInformationCollected() && (TlsHandshakeState::CLIENT_KEY_EXCHANGE == currentState)) {
		const auto randomPtr = impl->ssl.handshake->randbytes;
		const auto randomLen = sizeof(impl->ssl.handshake->randbytes);
		impl->clientRandom = std::vector<uint8_t>(randomPtr, randomPtr + randomLen / 2);
	}
	if (TlsMessage::isSent(isClient(), currentState)) {
		impl->tryToReadAlertMessage(false);
	}
	int result;
	while (0 != (result = mbedtls_ssl_handshake_step(&impl->ssl))) {
		if ((MBEDTLS_ERR_SSL_WANT_READ != result) && (MBEDTLS_ERR_SSL_WANT_WRITE != result)) {
			impl->tryToReadAlertMessage(true);
			throw std::runtime_error("mbedtls_ssl_handshake_step failed: " + errorToString(result));
		}
	}
	if (expectingPeerFinishedMessage) {
		const auto ivLen = impl->ssl.transform_in->ivlen;
		const auto ivPtr = impl->ssl.transform_in->iv_dec;
		const auto ivStr = Tooling::HexStringHelper::byteArrayToHexString({ivPtr, ivPtr + ivLen});
		log(__FILE__, __LINE__, std::string{"Finished.GenericBlockCipher.IV="} + ivStr);
	}
	if (isSecrectInformationCollected() && !impl->clientRandom.empty() && expectingPeerFinishedMessage) {
		const auto masterPtr = impl->ssl.session_negotiate->master;
		const auto masterLen = sizeof(impl->ssl.session_negotiate->master);
		provideSecrectInformation(impl->clientRandom, {masterPtr, masterPtr + masterLen});
	}
	impl->waitForExpectedAlertMessage(TlsMessage::isSent(isClient(), currentState));
	onPostStep();
}

void TlsSession::sendApplicationData(const std::vector<uint8_t> & data) {
	std::size_t length = data.size();
	int result;
	for (std::size_t written = 0; written < length; written += result) {
		while (0 >= (result = mbedtls_ssl_write(&impl->ssl, data.data() + written, length - written))) {
			if ((MBEDTLS_ERR_SSL_WANT_READ != result) && (MBEDTLS_ERR_SSL_WANT_WRITE != result)) {
				throw std::runtime_error("mbedtls_ssl_write failed: " + errorToString(result));
			}
		}
	}
	impl->waitForExpectedAlertMessage(true);
}

std::vector<uint8_t> TlsSession::receiveApplicationData() {
	return impl->receiveApplicationData();
}


void TlsSession::close(const uint32_t closeTimeout) {
	log(__FILE__, __LINE__, "Closing the TLS session.");
	if (closeTimeout != 0) {
		static const std::chrono::seconds timeout(closeTimeout);
		const auto timeStart = std::chrono::steady_clock::now();
		while (!getSocket().isClosed()) {
			impl->tryToReadAlertMessage(true);
			if (timeout < (std::chrono::steady_clock::now() - timeStart)) {
				break;
			}
		}
	}
	int result;
	do {
		result = mbedtls_ssl_close_notify(&impl->ssl);
	} while (MBEDTLS_ERR_SSL_WANT_WRITE == result);
	impl->tryToReadAlertMessage(true);
}

TlsHandshakeState TlsSession::getState() const {
	return convertState(impl->ssl.state);
}

void TlsSession::setState(TlsHandshakeState manipulatedState) {
	impl->ssl.state = convertState(manipulatedState);
	impl->expectAlertMessage = true;
}

TlsVersion TlsSession::getVersion() const {
	return std::make_pair(impl->ssl.major_ver, impl->ssl.minor_ver);
}

void TlsSession::setVersion(const TlsVersion & version) {
	mbedtls_ssl_conf_min_version(&impl->conf, version.first, version.second);
	mbedtls_ssl_conf_max_version(&impl->conf, version.first, version.second);
}

void TlsSession::setCipherSuites(const std::vector<TlsCipherSuite> & cipherSuites) {
	impl->tlsCipherSuites.resize(cipherSuites.size() + 1);
	std::transform(cipherSuites.cbegin(), cipherSuites.cend(), impl->tlsCipherSuites.begin(),
				   [](const TlsCipherSuite & cipherSuite) { return (cipherSuite.first << 8) | cipherSuite.second; });
	// Zero terminator as last element
	impl->tlsCipherSuites.back() = 0;
	mbedtls_ssl_conf_ciphersuites(&impl->conf, impl->tlsCipherSuites.data());
}

void TlsSession::overwritePreMasterSecretVersion(const TlsVersion & newValue) {
	impl->conf.overwrite_pms_version = 1;
	impl->conf.pms_version[0] = newValue.first;
	impl->conf.pms_version[1] = newValue.second;
}

void TlsSession::overwritePreMasterSecretRandom() {
	impl->conf.overwrite_pms_random = 1;
}

void TlsSession::overwritePreMasterSecretRandomByte(uint16_t index) {
	impl->conf.overwrite_pms_random_byte = 1;
	impl->conf.pms_random_byte_index = index;
}

static mbedtls_rsa_context * getPeerCertificateRsaContext(mbedtls_ssl_context & ssl) {
	if ((nullptr == ssl.session_negotiate->peer_cert) || (nullptr == ssl.session_negotiate->peer_cert->pk.pk_info)
		|| (MBEDTLS_PK_RSA != ssl.session_negotiate->peer_cert->pk.pk_info->type)
		|| (nullptr == ssl.session_negotiate->peer_cert->pk.pk_ctx)) {
		return nullptr;
	}
	return mbedtls_pk_rsa(ssl.session_negotiate->peer_cert->pk);
}

static void wrongRsaesPkcs1V15EncryptPadding(mbedtls_ssl_context & ssl, unsigned int use_wrong_padding,
											 unsigned char wrong_first_byte_value,
											 unsigned char wrong_block_type_value,
											 unsigned char wrong_padding_value) {
	auto rsaContext = getPeerCertificateRsaContext(ssl);
	if (nullptr == rsaContext) {
		return;
	}
	rsaContext->use_wrong_padding = use_wrong_padding;
	rsaContext->wrong_first_byte_value = wrong_first_byte_value;
	rsaContext->wrong_block_type_value = wrong_block_type_value;
	rsaContext->wrong_padding_value = wrong_padding_value;
}

void TlsSession::overwriteRsaesPkcs1V15EncryptPadding(const uint8_t first_byte,
													  const uint8_t block_type,
													  const uint8_t padding) {
	wrongRsaesPkcs1V15EncryptPadding(impl->ssl, 1, first_byte, block_type, padding);
}

void TlsSession::restoreRsaesPkcs1V15EncryptPadding() {
	wrongRsaesPkcs1V15EncryptPadding(impl->ssl, 0, 0x00, 0x02, 0x00);
}

void TlsSession::skipRsaesPkcs1V15EncryptPaddingCheck(const bool skipFirstByte,
												  const bool skipBlockType,
												  const bool skipDelimiter,
												  const bool skipPmsVersion) {
	if ((nullptr == impl->privateKey.pk_info)
			|| (MBEDTLS_PK_RSA != impl->privateKey.pk_info->type)
			|| (nullptr == impl->privateKey.pk_ctx)) {
		return;
	}

	auto rsaContext = mbedtls_pk_rsa(impl->privateKey);
	if (nullptr == rsaContext) {
		return;
	}

	if (skipFirstByte) {
		rsaContext->skip_first_byte_check = 1;
	}
	if (skipBlockType) {
		rsaContext->skip_block_type_check = 1;
	}
	if (skipDelimiter) {
		rsaContext->skip_padding_value_check = 1;
	}
	if (skipPmsVersion) {
		impl->conf.skip_pms_version_check = 1;
	}
}

void TlsSession::restoreRsaesPkcs1V15EncryptPaddingCheck() {
	if ((nullptr == impl->privateKey.pk_info)
			|| (MBEDTLS_PK_RSA != impl->privateKey.pk_info->type)
			|| (nullptr == impl->privateKey.pk_ctx)) {
		return;
	}

	auto rsaContext = mbedtls_pk_rsa(impl->privateKey);
	if (nullptr == rsaContext) {
		return;
	}

	rsaContext->skip_first_byte_check = 0;
	rsaContext->skip_block_type_check = 0;
	rsaContext->skip_padding_value_check = 0;
	impl->conf.skip_pms_version_check = 0;
}


void TlsSession::setLogger(Tooling::Logger & logger) {
	TlsTestTool::TlsSession::setLogger(logger);
	TlsLogFilter::registerInstances(logger);
	mbedtls_debug_set_threshold(9999);
	mbedtls_ssl_conf_dbg(&impl->conf, &writeToLogger, &logger);
	log(__FILE__, __LINE__, "Using patched " MBEDTLS_VERSION_STRING_FULL ".");
}

void TlsSession::setWaitForAlertSeconds(uint32_t timeout) {
	impl->waitForAlertSeconds = timeout;
}

void TlsSession::setTcpReceiveTimeoutSeconds(uint32_t timeout) {
	impl->tcpReceiveTimeoutSeconds = timeout;
}

void TlsSession::setServerSimulation(uint16_t id) {
	impl->conf.server_simulation_id = id;
	if (id != 0) {
		std::string simulatedServer;
		switch ( id )
		{
		   case 1:
			  simulatedServer = "CISCO ACE";
			  break;
		   case 2:
			  simulatedServer = "Facebook v2";
			  break;
		   case 3:
			  simulatedServer = "F5 v1";
		      break;
		   case 4:
			  simulatedServer = "PAN OS";
		      break;
		   case 5:
		      simulatedServer = "Netscaler GCM";
		      break;
           case 6:
              simulatedServer = "Delay";
              break;
		   default:
			  simulatedServer = "Unknown";
		}
		log(__FILE__, __LINE__, "Using server simulation " + simulatedServer + ".");
	}
}

void TlsSession::setServerSimulationDelay(const useconds_t delayInMicrosecond) {
    impl->conf.server_simulation_delay = delayInMicrosecond;
    log(__FILE__, __LINE__, "Using server simulation delay of " + std::to_string(delayInMicrosecond) + " microseconds.");
}

}
}
