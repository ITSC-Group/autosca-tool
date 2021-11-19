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
#include "logging/Logger.h"
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

namespace TlsTestTool {

class TlsSession::Data {
public:
	Tooling::Logger * logger;
	TcpClient * const tcpClient;
	TcpServer * const tcpServer;
	TlsCallbackFunction preStepCallback;
	TlsCallbackFunction postStepCallback;
	std::unique_ptr<std::ostream> secretOutput;

	Data(TcpClient * client, TcpServer * server)
			: logger(nullptr),
			  tcpClient(client),
			  tcpServer(server),
			  preStepCallback(),
			  postStepCallback(),
			  secretOutput() {
	}

	bool isClient() const {
		return (nullptr != tcpClient);
	}

	TcpClient & getSocket() {
		return isClient() ? *tcpClient : tcpServer->getClient();
	}

	void log(const std::string & file, const int line, const std::string & message) {
		if (nullptr != logger) {
			logger->log(Tooling::LogLevel::HIGH, "TLS", file, line, message);
		}
	}
};

TlsSession::TlsSession(TcpClient & tcpClient) : impl(std::make_unique<Data>(&tcpClient, nullptr)) {
}

TlsSession::TlsSession(TcpServer & tcpServer) : impl(std::make_unique<Data>(nullptr, &tcpServer)) {
}

TlsSession::~TlsSession() = default;

TcpClient & TlsSession::getSocket() {
	return impl->getSocket();
}

bool TlsSession::isClient() const {
	return impl->isClient();
}

void TlsSession::setLogger(Tooling::Logger & logger) {
	impl->logger = &logger;
}

void TlsSession::setSecretOutput(std::unique_ptr<std::ostream> && output) {
	impl->secretOutput = std::move(output);
}

bool TlsSession::isSecrectInformationCollected() {
	return impl->secretOutput.operator bool();
}

void TlsSession::provideSecrectInformation(const std::vector<uint8_t> & clientRandom,
										   const std::vector<uint8_t> & masterSecret) {
	if (impl->secretOutput) {
		// https://developer.mozilla.org/en-US/docs/Mozilla/Projects/NSS/Key_Log_Format
		auto strRandom = Tooling::HexStringHelper::byteArrayToHexString(clientRandom);
		strRandom.erase(std::remove(strRandom.begin(), strRandom.end(), ' '), strRandom.end());
		auto strMaster = Tooling::HexStringHelper::byteArrayToHexString(masterSecret);
		strMaster.erase(std::remove(strMaster.begin(), strMaster.end(), ' '), strMaster.end());
		*(impl->secretOutput) << "CLIENT_RANDOM " << strRandom << ' ' << strMaster << std::endl;
	}
}

void TlsSession::registerPreStepCallback(TlsCallbackFunction && callback) {
	impl->preStepCallback = std::move(callback);
}

void TlsSession::onPreStep() {
	if (impl->preStepCallback) {
		impl->preStepCallback(*this);
	}
}

void TlsSession::registerPostStepCallback(TlsCallbackFunction && callback) {
	impl->postStepCallback = std::move(callback);
}

void TlsSession::onPostStep() {
	if (impl->postStepCallback) {
		impl->postStepCallback(*this);
	}
}

void TlsSession::log(const std::string & file, const int line, const std::string & message) {
	impl->log(file, line, message);
}
}
