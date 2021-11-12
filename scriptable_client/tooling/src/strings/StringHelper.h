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
#ifndef STRINGS_STRINGHELPER_H_
#define STRINGS_STRINGHELPER_H_

#include <algorithm>
#include <cstdio>
#include <sstream>
#include <string>
#include <vector>

namespace Tooling {
/**
 * Collection of helper functions that deal with strings.
 */
namespace StringHelper {
/**
 * Print a formatted string with one integer parameter. This is a wrapper for @link std::snprintf @endlink that creates
 * a string that holds the output.
 *
 * @param formatString See parameter @c format of @link std::snprintf @endlink
 * @param number Integer that will be given to @link std::snprintf @endlink
 * @return String containing the output of @link std::snprintf @endlink
 */
template <typename T> inline std::string formatInt(const std::string & formatString, const T number) {
	const auto requiredSize = std::snprintf(nullptr, 0, formatString.c_str(), number);
	std::vector<char> outputStr(requiredSize + 1);
	std::snprintf(&outputStr[0], outputStr.size(), formatString.c_str(), number);
	return {outputStr.data(), outputStr.size() - 1};
}

/**
 * Remove newline characters '\\r' and '\\n' from a string. Replace every '\\n' by a space character.
 *
 * @param str String that will be stripped of newlines
 * @return String without newlines
 */
inline std::string removeNewlines(std::string && str) {
	str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
	std::replace(str.begin(), str.end(), '\n', ' ');
	return str;
}

/**
 * Remove whitespace at the beginning and the end of a string.
 *
 * @param str String that will be trimmed
 * @return String with whitespace at beginning and end removed
 */
inline std::string trim(const std::string & str) {
	const std::string whiteSpace{" \t\v\f\r\n"};
	const auto startPos = str.find_first_not_of(whiteSpace);
	const auto endPos = str.find_last_not_of(whiteSpace);
	if ((std::string::npos == startPos) || (std::string::npos == endPos)) {
		return {};
	} else {
		return str.substr(startPos, endPos - startPos + 1);
	}
}

}
}

#endif /* STRINGS_STRINGHELPER_H_ */
