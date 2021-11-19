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
#include "ConfigurationParser.h"
#include "configuration/Configuration.h"
#include "manipulation/ManipulationsParser.h"
#include <regex>
#include <stdexcept>
#include <string>

namespace TlsTestTool {
static const std::string manipulatePrefix{"manipulate"};

void ConfigurationParser::updateConfiguration(Configuration & configuration,
											  const std::vector<Tooling::KeyValuePair> & keyValuePairs) {
    bool isTlsServerSimulationDelaySet = false;
    bool isTlsServerSimulationDelayEnabled = false;

	for (const auto & keyValuePair : keyValuePairs) {
		const std::string & name = keyValuePair.first;
		const std::string & value = keyValuePair.second;
		if (name == "mode") {
			if (value == "client") {
				configuration.setMode(Configuration::NetworkMode::CLIENT);
			} else if (value == "server") {
				configuration.setMode(Configuration::NetworkMode::SERVER);
			} else {
				throw std::invalid_argument{std::string{"Unknown mode "} + value};
			}
        } else if (name == "host") {
			configuration.setHost(value);
		} else if (name == "port") {
			configuration.setPort(std::stoul(value));
		} else if (name == "listenTimeout") {
			configuration.setListenTimeoutSeconds(std::stoul(value));
		} else if (name == "waitBeforeClose") {
			configuration.setWaitBeforeCloseSeconds(std::stoul(value));
		} else if (name == "receiveTimeout") {
			configuration.setTcpReceiveTimeoutSeconds(std::stoul(value));
		} else if (name == "certificateFile") {
			configuration.setCertificateFile(value);
		} else if (name == "privateKeyFile") {
			configuration.setPrivateKeyFile(value);
		} else if (name == "tlsVersion") {
			const std::regex numberPairRegEx{"\\(([0-9]+),([0-9]+)\\)"};
			std::smatch valueMatch;
			if (std::regex_match(value, valueMatch, numberPairRegEx)) {
				if (3 != valueMatch.size()) {
					throw std::invalid_argument{std::string{"Invalid value for "} + name + " " + value};
				}
				const auto major = std::stoul(valueMatch[1]);
				const auto minor = std::stoul(valueMatch[2]);
				if (3 != major) {
					throw std::invalid_argument{std::string{"Invalid major version for "} + name + " " + value};
				}
				if ((0 == minor) || (3 < minor)) {
					throw std::invalid_argument{std::string{"Invalid minor version for "} + name + " " + value};
				}
				configuration.setTlsVersion(std::make_pair(static_cast<uint8_t>(major), static_cast<uint8_t>(minor)));
			} else {
				throw std::invalid_argument{std::string{"Invalid value for "} + name + " " + value};
			}
		} else if (name == "tlsCipherSuites") {
			const std::regex hexPairRegEx{"\\((0x[0-9a-fA-F]{2}),(0x[0-9a-fA-F]{2})\\)"};
			auto hexPairsBegin = std::sregex_iterator(value.begin(), value.end(), hexPairRegEx);
			auto hexPairsEnd = std::sregex_iterator();
			if (0 == std::distance(hexPairsBegin, hexPairsEnd)) {
				throw std::invalid_argument{std::string{"Invalid value for "} + name + " " + value};
			}
			configuration.clearTlsCipherSuites();
			for (auto hexPair = hexPairsBegin; hexPair != hexPairsEnd; ++hexPair) {
				auto valueMatch = *hexPair;
				if (3 != valueMatch.size()) {
					throw std::invalid_argument{std::string{"Invalid value for "} + name + " " + value};
				}
				const std::string upperStr{valueMatch[1]};
				const auto upper = std::stoul(upperStr, 0, 16);
				if (255 < upper) {
					throw std::invalid_argument{std::string{"Invalid upper byte "} + upperStr + " for " + name + " in "
												+ value};
				}
				const std::string lowerStr{valueMatch[2]};
				const auto lower = std::stoul(lowerStr, 0, 16);
				if (255 < lower) {
					throw std::invalid_argument{std::string{"Invalid lower byte "} + lowerStr + " for " + name + " in "
												+ value};
				}
				configuration.addTlsCipherSuite(
						std::make_pair(static_cast<uint8_t>(upper), static_cast<uint8_t>(lower)));
			}
		} else if (name == "tlsSecretFile") {
			configuration.setTlsSecretFile(value);
		} else if (name.substr(0, manipulatePrefix.size()) == manipulatePrefix) {
			ManipulationsParser::parse(name, value, configuration);
		} else if (name == "tlsServerSimulation") {
			const auto id = std::stoul(value);
            if (id > 6) {
				throw std::invalid_argument{std::string{"Invalid value for "} + name + " " + value};
			}
            else if (id == 6){
                isTlsServerSimulationDelayEnabled = true;
            }
			configuration.setServerSimulation(id);
        } else if (name == "tlsServerSimulationDelay") {
            char *ptr;
            const auto delay = std::strtoul(value.c_str(),&ptr,10);
            if (delay > 1000000) {
                throw std::invalid_argument{std::string{"Invalid value for "} + name + " " + value};
            }
            configuration.setServerSimulationDelay(delay);
            isTlsServerSimulationDelaySet = true;
        } else {
			throw std::invalid_argument{std::string{"Unknown argument "} + name};
		}
	}
    if(isTlsServerSimulationDelayEnabled && !isTlsServerSimulationDelaySet){
        throw std::invalid_argument{std::string{"The value of 'tlsServerSimulationDelay' needs to be set for tlsServerSimulation mode 6"}};
    }
}
}
