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
#include "ManipulationsParser.h"
#include "ManipulatePreMasterSecretRandom.h"
#include "ManipulatePreMasterSecretRandomByte.h"
#include "ManipulatePreMasterSecretVersion.h"
#include "ManipulateRsaesPkcs1V15EncryptPadding.h"
#include "SkipChangeCipherSpec.h"
#include "SkipFinished.h"
#include "SkipRsaesPkcs1V15PaddingCheck.h"
#include "configuration/Configuration.h"
#include "strings/HexStringHelper.h"
#include "tls/TlsHashAlgorithm.h"
#include <cstdint>
#include <limits>
#include <regex>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>

namespace TlsTestTool {
static bool stob(const std::string & value) {
	if (value == "true") {
		return true;
	} else if (value == "false") {
		return false;
	} else {
		throw std::invalid_argument{std::string{"Invalid value \""} + value + "\" for boolean conversion"};
	}
}

static std::smatch matchValue(const std::string & name, const std::string & value, const std::regex & regEx) {
	std::smatch valueMatch;
	if (std::regex_match(value, valueMatch, regEx)) {
		if ((regEx.mark_count() + 1) != valueMatch.size()) {
			throw std::invalid_argument{std::string{"Invalid value \""} + value + "\" for " + name};
		}
		return valueMatch;
	} else {
		throw std::invalid_argument{std::string{"Invalid value \""} + value + "\" for " + name};
	}
}

static uint8_t matchByte(const std::string & name, const std::string & value, const std::string & byteString) {
	const auto longValue = std::stoul(byteString, 0, 16);
	if (std::numeric_limits<uint8_t>::max() < longValue) {
		throw std::invalid_argument{std::string{"Invalid byte \""} + byteString + "\" in value \"" + value + "\" for "
									+ name};
	}
	return longValue;
}

static std::pair<uint8_t, uint8_t> matchHexPair(const std::string & name, const std::string & value) {
	const std::regex hexPairRegEx{"\\((0x[0-9a-fA-F]{2}),(0x[0-9a-fA-F]{2})\\)"};
	const auto valueMatch = matchValue(name, value, hexPairRegEx);
	const auto firstByte = matchByte(name, value, valueMatch[1]);
	const auto secondByte = matchByte(name, value, valueMatch[2]);
	return std::make_pair(firstByte, secondByte);
}

void ManipulationsParser::parse(const std::string & name, const std::string & value, Configuration & configuration) {
	if (name == "manipulateSkipChangeCipherSpec") {
		configuration.addManipulation(std::make_unique<SkipChangeCipherSpec>());
	} else if (name == "manipulateSkipFinished") {
		configuration.addManipulation(std::make_unique<SkipFinished>());
	} else if (name == "manipulatePreMasterSecretRandom") {
		configuration.addManipulation(std::make_unique<ManipulatePreMasterSecretRandom>());
	} else if (name == "manipulatePreMasterSecretRandomByte") {
		try {
			const auto index = std::stoull(value);
			if (index >= 46) {
				throw std::invalid_argument{std::string{"Invalid index \""} + value + "\" for " + name};
			}
			configuration.addManipulation(std::make_unique<ManipulatePreMasterSecretRandomByte>(index));
		} catch (const std::exception & e) {
			throw std::invalid_argument{std::string{"Invalid index \""} + value + "\" for " + name};
		}
	} else if (name == "manipulateRsaesPkcs1V15EncryptPadding") {
		const std::regex hexTripleRegEx{"(0x[0-9a-fA-F]{2}),(0x[0-9a-fA-F]{2}),(0x[0-9a-fA-F]{2})"};
		const auto valueMatch = matchValue(name, value, hexTripleRegEx);
		const auto firstByte = matchByte(name, value, valueMatch[1]);
		const auto blockType = matchByte(name, value, valueMatch[2]);
		const auto padding = matchByte(name, value, valueMatch[3]);
		configuration.addManipulation(std::make_unique<ManipulateRsaesPkcs1V15EncryptPadding>(firstByte, blockType, padding));
	} else if (name == "manipulatePreMasterSecretVersion") {
		const auto manipulatedVersion = matchHexPair(name, value);
		configuration.addManipulation(std::make_unique<ManipulatePreMasterSecretVersion>(manipulatedVersion));
	} else if (name == "manipulateSkipRsaesPkcs1V15PaddingCheck") {
		const std::regex boolQuadrupleRegEx{"(true|false),(true|false),(true|false),(true|false)"};
		const auto valueMatch = matchValue(name, value, boolQuadrupleRegEx);
		const auto skipFirstByteCheck = stob(valueMatch[1]);
		const auto skipBlockTypeCheck = stob(valueMatch[2]);;
		const auto skipDelimiterCheck = stob(valueMatch[3]);;
		const auto skipPmsVersionCheck = stob(valueMatch[4]);;
		configuration.addManipulation(std::make_unique<SkipRsaesPkcs1V15PaddingCheck>(skipFirstByteCheck,
																					  skipBlockTypeCheck,
																					  skipDelimiterCheck,
																					  skipPmsVersionCheck));
	} else {
		throw std::invalid_argument{std::string{"Unknown manipulation "} + name};
	}
}
}
