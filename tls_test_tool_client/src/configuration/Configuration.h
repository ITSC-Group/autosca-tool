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
#ifndef CONFIGURATION_CONFIGURATION_H_
#define CONFIGURATION_CONFIGURATION_H_

#include "logging/LogLevel.h"
#include "manipulation/Manipulation.h"
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace TlsTestTool {
/**
 * Configuration description container.
 */
class Configuration {
public:
	enum class NetworkMode {
		//! Run the test tool as TCP/IP client.
		CLIENT,
		//! Run the test tool as TCP/IP server.
		SERVER,
		//! Mode has not been set.
		UNKNOWN
	};

    enum class TlsLibrary {
        //! Use mbed TLS as TLS library.
        MBED_TLS,
        //! Use TLS-Attacker as TLS library.
        TLS_ATTACKER,
        //! TLS implementstion has not been set.
        UNKNOWN
    };

    Configuration()
            : mode(NetworkMode::UNKNOWN),
               tlsLibrary(TlsLibrary::MBED_TLS),
			  host(),
			  port(0),
 			  listenTimeoutSeconds(0),
			  waitBeforeCloseSeconds(10000),
			  tcpReceiveTimeoutSeconds(120000),
 			  logLevel(Tooling::LogLevel::LOW),
			  certificateFile(),
			  privateKeyFile(),
 			  tlsVersion(std::make_pair(0, 0)),
			  tlsServerSimulation(0),
			  manipulations() {
	}

	NetworkMode getMode() const {
		return mode;
	}

	void setMode(const NetworkMode newMode) {
		mode = newMode;
	}

    TlsLibrary getTlsLibrary() const {
        return tlsLibrary;
    }

    void setTlsLibrary(const TlsLibrary newTlsLibrary) {
        tlsLibrary = newTlsLibrary;
    }

    const std::string & getHost() const {
		return host;
	}

	void setHost(const std::string & newHost) {
		host = newHost;
	}

	uint16_t getPort() const {
		return port;
	}

	void setPort(const uint16_t newPort) {
		port = newPort;
	}

	uint32_t getListenTimeoutSeconds() const {
		return listenTimeoutSeconds;
	}

	void setListenTimeoutSeconds(const uint32_t newListenTimeoutSeconds) {
		listenTimeoutSeconds = newListenTimeoutSeconds;
	}

	uint32_t getWaitBeforeCloseSeconds() const {
		return waitBeforeCloseSeconds;
	}

	void setWaitBeforeCloseSeconds(const uint32_t newWaitBeforeCloseSeconds) {
		waitBeforeCloseSeconds = newWaitBeforeCloseSeconds;
	}

	uint32_t getTcpReceiveTimeoutSeconds() const {
		return tcpReceiveTimeoutSeconds;
	}

	void setTcpReceiveTimeoutSeconds(const uint32_t newtcpReceiveTimeoutSeconds) {
		tcpReceiveTimeoutSeconds = newtcpReceiveTimeoutSeconds;
	}

	Tooling::LogLevel getLogLevel() const {
		return logLevel;
	}

	void setLogLevel(const Tooling::LogLevel newLogLevel) {
		logLevel = newLogLevel;
	}

	const std::string & getCertificateFile() const {
		return certificateFile;
	}

	void setCertificateFile(const std::string & newCertificateFile) {
		certificateFile = newCertificateFile;
	}

	const std::string & getPrivateKeyFile() const {
		return privateKeyFile;
	}

	void setPrivateKeyFile(const std::string & newPrivateKeyFile) {
		privateKeyFile = newPrivateKeyFile;
	}

	bool hasTlsVersion() const {
		return std::make_pair(static_cast<uint8_t>(0), static_cast<uint8_t>(0)) != tlsVersion;
	}

	const std::pair<uint8_t, uint8_t> & getTlsVersion() const {
		return tlsVersion;
	}

	void setTlsVersion(const std::pair<uint8_t, uint8_t> & newTlsVersion) {
		tlsVersion = newTlsVersion;
	}

	bool hasTlsCipherSuites() const {
		return !tlsCipherSuites.empty();
	}

	const std::vector<std::pair<uint8_t, uint8_t>> & getTlsCipherSuites() const {
		return tlsCipherSuites;
	}

	void clearTlsCipherSuites() {
		tlsCipherSuites.clear();
	}

	void addTlsCipherSuite(std::pair<uint8_t, uint8_t> && tlsCipherSuite) {
		tlsCipherSuites.emplace_back(std::move(tlsCipherSuite));
	}

	const std::vector<std::unique_ptr<Manipulation>> & getManipulations() const {
		return manipulations;
	}

	void addManipulation(std::unique_ptr<Manipulation> && manipulation) {
		manipulations.emplace_back(std::move(manipulation));
	}

	const std::string & getTlsSecretFile() const {
		return tlsSecretFile;
	}

	void setTlsSecretFile(const std::string & newTlsSecretFile) {
		tlsSecretFile = newTlsSecretFile;
	}

	uint16_t getServerSimulation() const {
		return tlsServerSimulation;
	}

	void setServerSimulation(const uint16_t newServerSimulation) {
		tlsServerSimulation = newServerSimulation;
	}

    useconds_t getServerSimulationDelay() const {
        return tlsServerSimulationDelay;
    }

    void setServerSimulationDelay(const useconds_t newServerSimulationDelay) {
        tlsServerSimulationDelay = newServerSimulationDelay;
    }

private:
	NetworkMode mode;
    TlsLibrary tlsLibrary;
	std::string host;
	uint16_t port;
	uint32_t listenTimeoutSeconds;
	uint32_t waitBeforeCloseSeconds;
	uint32_t tcpReceiveTimeoutSeconds;
	Tooling::LogLevel logLevel;
	std::string certificateFile;
	std::string privateKeyFile;
	std::pair<uint8_t, uint8_t> tlsVersion;
	std::vector<std::pair<uint8_t, uint8_t>> tlsCipherSuites;
	std::string tlsSecretFile;
	uint16_t tlsServerSimulation;
    useconds_t tlsServerSimulationDelay;
	std::vector<std::unique_ptr<Manipulation>> manipulations;
};
}

#endif /* CONFIGURATION_CONFIGURATION_H_ */
