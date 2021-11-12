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
#ifndef CONFIGURATION_CONFIGURATIONFILEPARSER_H_
#define CONFIGURATION_CONFIGURATIONFILEPARSER_H_

#include "configuration/KeyValuePair.h"
#include <iosfwd>
#include <vector>

namespace Tooling {
/**
 * Parser for configuration arguments stored line by line in a configuration file.
 */
class ConfigurationFileParser {
public:
	/**
	 * Factory function parsing configuration arguments and creating an array of key-value pairs. Arguments of the form
	 * "key=value" are accepted. Empty lines and lines starting with '#' are ignored.
	 *
	 * @param input Input stream containing the data to parse
	 * @return Array of key-value pairs
	 * @throw std::exception Thrown, if a required argument is missing, or a error occurred during reading.
	 */
	static std::vector<KeyValuePair> parse(std::istream & input);
};
}

#endif /* CONFIGURATION_CONFIGURATIONFILEPARSER_H_ */
