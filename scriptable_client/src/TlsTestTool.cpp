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
#include "configuration/Configuration.h"
#include "configuration/ConfigurationLoader.h"
#include "logging/LogLevel.h"
#include "logging/Logger.h"
#include "network/TcpClient.h"
#include "network/TcpServer.h"
#include "network/TimestampObserver.h"
#include "strings/HexStringHelper.h"
#include "strings/StringHelper.h"
#include "tls/TlsSession.h"
#include "tls/TlsSessionFactory.h"
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>

#ifndef TLS_TEST_TOOL_VERSION
#define TLS_TEST_TOOL_VERSION "UNRELEASED"
#endif /* TLS_TEST_TOOL_VERSION */

static void logException(Tooling::Logger & logger, const std::string & category, const std::string & file,
						 const int line, const std::string & message, const std::exception & e) {
	logger.log(Tooling::LogLevel::HIGH, category, file, line,
			   message + ": " + Tooling::StringHelper::removeNewlines(e.what()));
}

static bool configureTlsSession(const TlsTestTool::Configuration & configuration, TlsTestTool::TlsSession & tlsSession,
								Tooling::Logger & logger) {
	if (configuration.hasTlsVersion()) {
		tlsSession.setVersion(configuration.getTlsVersion());
	}

	if (configuration.hasTlsCipherSuites()) {
		tlsSession.setCipherSuites(configuration.getTlsCipherSuites());
	}
	if (!configuration.getTlsSecretFile().empty()) {
		try {
			tlsSession.setSecretOutput(std::make_unique<std::ofstream>(configuration.getTlsSecretFile(), std::ios::out | std::ios::app));
		} catch (const std::exception & e) {
			logException(logger, "TLS", __FILE__, __LINE__, "Configuring TLS secret file failed", e);
			return false;
		}
	}
	tlsSession.setWaitForAlertSeconds(configuration.getWaitBeforeCloseSeconds());
	tlsSession.setTcpReceiveTimeoutSeconds(configuration.getTcpReceiveTimeoutSeconds());
	tlsSession.setServerSimulation(configuration.getServerSimulation());
    if(configuration.getServerSimulation() == 6){
        tlsSession.setServerSimulationDelay(configuration.getServerSimulationDelay());
    }
	return true;
}

static bool configureCertificates(const TlsTestTool::Configuration & configuration,
								  TlsTestTool::TlsSession & tlsSession, Tooling::Logger & logger) {
	if (!configuration.getCertificateFile().empty() && !configuration.getPrivateKeyFile().empty()) {
		try {
			std::ifstream certificateFile(configuration.getCertificateFile());
			std::ifstream privateKeyFile(configuration.getPrivateKeyFile());
			tlsSession.setCertificate(certificateFile, privateKeyFile);
		} catch (const std::exception & e) {
			logException(logger, "TLS", __FILE__, __LINE__, "Loading certificate and private key files failed", e);
			return false;
		}
	}
	return true;
}

static bool checkTcpConnection(TlsTestTool::TcpClient & socket, Tooling::Logger & logger) {
	if (socket.isClosed()) {
		logger.log(Tooling::LogLevel::HIGH, "Network", __FILE__, __LINE__, "TCP/IP connection is closed.");
		return false;
	} else {
		return true;
	}
}

static bool checkTcpConnectionPollOne(TlsTestTool::TcpClient & socket) {
	if (socket.isClosed(true)) {
		return false;
	} else {
		return true;
	}
}

static void waitForClosedTcpConnection(const TlsTestTool::Configuration & configuration, TlsTestTool::TcpClient & socket, Tooling::Logger & logger) {
	static const std::chrono::milliseconds timeout(configuration.getWaitBeforeCloseSeconds());
	logger.log(Tooling::LogLevel::HIGH, "Network", __FILE__, __LINE__,
			   "Wait at most " + std::to_string(timeout.count()) + " s for closing of the TCP/IP connection.");
	const auto timeStart = std::chrono::steady_clock::now();
	while (checkTcpConnection(socket, logger)) {
		if (timeout < (std::chrono::steady_clock::now() - timeStart)) {
			logger.log(Tooling::LogLevel::HIGH, "Network", __FILE__, __LINE__, "TCP/IP connection is still open.");
			break;
		}
	}
}

static void configureCallbacks(const TlsTestTool::Configuration & configuration, TlsTestTool::TlsSession & tlsSession,
							   TlsTestTool::TcpClient & client) {
	tlsSession.registerPreStepCallback([&](TlsTestTool::TlsSession & session) {
		for (auto & manipulation : configuration.getManipulations()) {
			manipulation->executePreStep(session);
		}
		// It is necessary to check the connection so that the handlers that are ready to run are executed
		// in the isClosed function with poll() or poll_one().
		checkTcpConnectionPollOne(client);
	});
	tlsSession.registerPostStepCallback([&](TlsTestTool::TlsSession & session) {
		for (auto & manipulation : configuration.getManipulations()) {
			manipulation->executePostStep(session);
		}
		// It is necessary to check the connection so that the handlers that are ready to run are executed
		// in the isClosed function with poll() or poll_one().
		checkTcpConnectionPollOne(client);
	});
}

static bool prepareTlsSession(const TlsTestTool::Configuration & configuration, TlsTestTool::TlsSession & tlsSession,
							  TlsTestTool::TcpClient & socket, Tooling::Logger & logger) {
	tlsSession.setLogger(logger);
	if (!configureTlsSession(configuration, tlsSession, logger)) {
		return false;
	}
	configureCallbacks(configuration, tlsSession, socket);
	if (!configureCertificates(configuration, tlsSession, logger)) {
		return false;
	}
	return true;
}

static void executeTlsSession(const TlsTestTool::Configuration & configuration, TlsTestTool::TlsSession & tlsSession,
							  TlsTestTool::TcpClient & socket, Tooling::Logger & logger, const uint32_t socketTimeout, const uint32_t closeTimeout) {
	TlsTestTool::TimestampObserver timestampObserver(socket, logger);
	socket.registerObserver(timestampObserver);
	try {
		for (auto & manipulation : configuration.getManipulations()) {
			manipulation->executePreHandshake(tlsSession);
		}
		tlsSession.performHandshake();
		for (auto & manipulation : configuration.getManipulations()) {
			manipulation->executePostHandshake(tlsSession);
		}
	} catch (const std::exception & e) {
		logException(logger, "TLS", __FILE__, __LINE__, "TLS handshake failed", e);
		waitForClosedTcpConnection(configuration, socket, logger);
		return;
	}
	if (!checkTcpConnection(socket, logger)) {
		return;
	}
	try {
		if (socketTimeout != 0) {
			static const std::chrono::seconds timeout(socketTimeout);
			const auto timeStart = std::chrono::steady_clock::now();
			while (0 == socket.available()) {
				if (timeout < (std::chrono::steady_clock::now() - timeStart)) {
					break;
				}
			}
		}
		if (0 < socket.available()) {
			const auto data = tlsSession.receiveApplicationData();
			logger.log(Tooling::LogLevel::HIGH, "TLS", __FILE__, __LINE__,
					   "Application data received: " + Tooling::HexStringHelper::byteArrayToHexString(data));
		}
	} catch (const std::exception & e) {
		logException(logger, "TLS", __FILE__, __LINE__, "Receiving application data failed", e);
		waitForClosedTcpConnection(configuration, socket, logger);
		return;
	}
	try {
		tlsSession.close(closeTimeout);
	} catch (const std::exception & e) {
		logException(logger, "TLS", __FILE__, __LINE__, "Closing failed", e);
		waitForClosedTcpConnection(configuration, socket, logger);
		return;
	}
	try {
		checkTcpConnection(socket, logger);
		socket.close();
	} catch (const std::exception & e) {
		logException(logger, "Network", __FILE__, __LINE__, "Closing failed", e);
		return;
	}
}

int main(int argc, char ** argv) {
	// Make sure not to flush stdout on '\n' for increased performance.
	std::cout.sync_with_stdio(false);
	Tooling::Logger logger(std::cout);
	logger.setColumnSeparator("\t");
	logger.setLogLevel(Tooling::LogLevel::HIGH);

	TlsTestTool::Configuration configuration;
	try {
		configuration = TlsTestTool::ConfigurationLoader::parse(argc, const_cast<const char **>(argv));
	} catch (const std::exception & e) {
		logException(logger, "Tool", __FILE__, __LINE__, "Parsing the configuration failed", e);
		return EXIT_FAILURE;
	}
	logger.log(Tooling::LogLevel::HIGH, "Tool", __FILE__, __LINE__, "TLS Test Tool version " TLS_TEST_TOOL_VERSION);
	logger.log(Tooling::LogLevel::HIGH, "Tool", __FILE__, __LINE__, "Copyright (C) 2016-2021 achelos GmbH");

	logger.setLogLevel(configuration.getLogLevel());
	for (auto & manipulation : configuration.getManipulations()) {
		manipulation->setLogger(logger);
	}
	
    TlsTestTool::Configuration::TlsLibrary tlsLibrary = configuration.getTlsLibrary();
    if (tlsLibrary == TlsTestTool::Configuration::TlsLibrary::UNKNOWN ) {
        logger.log(Tooling::LogLevel::HIGH, "Tool", __FILE__, __LINE__, "tlsLibrary not set, using MBED_TLS as default.");
        tlsLibrary = TlsTestTool::Configuration::TlsLibrary::MBED_TLS;
    }

	if (TlsTestTool::Configuration::NetworkMode::CLIENT == configuration.getMode()) {
		TlsTestTool::TcpClient client;
		try {
			client.connect(configuration.getHost(), std::to_string(configuration.getPort()));
			logger.log(Tooling::LogLevel::HIGH, "Network", __FILE__, __LINE__, "TCP/IP connection to "
							   + client.getRemoteIpAddress() + ':' + std::to_string(client.getRemoteTcpPort())
							   + " established.");
		} catch (const std::exception & e) {
			logException(logger, "Network", __FILE__, __LINE__, "TCP/IP connection to " + configuration.getHost() + ':'
								 + std::to_string(configuration.getPort()) + " failed",
						 e);
			return EXIT_FAILURE;
		}
        auto tlsSession = TlsTestTool::TlsSessionFactory::createClientSession(client);
		if (!prepareTlsSession(configuration, *tlsSession, client, logger)) {
			return EXIT_FAILURE;
		}
		executeTlsSession(configuration, *tlsSession, client, logger, 1, 3);
	} else if (TlsTestTool::Configuration::NetworkMode::SERVER == configuration.getMode()) {
		while (true) {
			TlsTestTool::TcpServer server;
			try {
				server.listen(configuration.getPort());
				logger.log(Tooling::LogLevel::HIGH, "Network", __FILE__, __LINE__,
						   "Waiting for TCP/IP connection on port " + std::to_string(configuration.getPort()) + '.');
			} catch (const std::exception & e) {
				logException(logger, "Network", __FILE__, __LINE__,
							 "Listening on port " + std::to_string(configuration.getPort()) + " failed", e);
				return EXIT_FAILURE;
			}

			auto tlsSession = TlsTestTool::TlsSessionFactory::createServerSession(server);
			if (!prepareTlsSession(configuration, *tlsSession, server.getClient(), logger)) {
				return EXIT_FAILURE;
			}
			static const std::chrono::seconds timeout(configuration.getListenTimeoutSeconds());
			const auto timeStart = std::chrono::steady_clock::now();
			// Flush output once before entering main loop.
			std::cout.flush();
			while (true) {
				server.work();
				if (!server.getClient().isClosed() && server.getClient().available()) {
					server.close();
					logger.log(Tooling::LogLevel::HIGH, "Network", __FILE__, __LINE__, "TCP/IP connection from "
									   + server.getClient().getRemoteIpAddress() + ':'
									   + std::to_string(server.getClient().getRemoteTcpPort()) + " received.");
					executeTlsSession(configuration, *tlsSession, server.getClient(), logger, 0, 0);
					break;
				}
				if ((0 != timeout.count()) && (timeout < (std::chrono::steady_clock::now() - timeStart))) {
					logger.log(Tooling::LogLevel::HIGH, "Network", __FILE__, __LINE__,
							   "Listen timeout after " + std::to_string(timeout.count()) + " s.");
					break;
				}
			}
		}
	}
	logger.log(Tooling::LogLevel::HIGH, "Tool", __FILE__, __LINE__, "TLS Test Tool exiting.");
	return EXIT_SUCCESS;
}
