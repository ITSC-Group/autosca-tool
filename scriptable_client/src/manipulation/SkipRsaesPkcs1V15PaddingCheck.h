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
#ifndef MANIPULATION_SKIPRSAESPKCS1V15PADDINGCHECK_H_
#define MANIPULATION_SKIPRSAESPKCS1V15PADDINGCHECK_H_

#include "Manipulation.h"
#include "tls/TlsHandshakeState.h"
#include <cstdint>

namespace TlsTestTool {
/**
 * Skip thecheck of the first byte, the block type byte, the delimiter byte and/or the pre-master secret version bytes
 * in RSAES-PKCS1-V1_5-ENCRYPT.
 * Encryption-block: firstByte = 00 || blockType = 02 || PS || delimiter = 00 || version = 0303 || M.
 * @see https://tools.ietf.org/html/rfc5246#section-7.4.7.1
 */
class SkipRsaesPkcs1V15PaddingCheck : public Manipulation {
public:
	/**
	 * Create a manipulation.
	 *
	 * @param skipFirstByteCheck First byte check will be skipped.
	 * @param skipBlockTypeCheck Block type byte check will be skipped.
	 * @param skipDelimiterCheck Padding byte check will be skipped.
	 * @param skipPmsVersionCheck PMS version bytes check will be skipped.
	 */
	SkipRsaesPkcs1V15PaddingCheck(const bool skipFirstByteCheck,
										  const bool skipBlockTypeCheck,
										  const bool skipDelimiterCheck,
								          const bool skipPmsVersionCheck) : Manipulation(),
		skipFirstByte(skipFirstByteCheck),
		skipBlockType(skipBlockTypeCheck),
		skipDelimiter(skipDelimiterCheck),
	    skipPmsVersion(skipPmsVersionCheck) {
	}

	bool getSkipFirstByte() const {
		return skipFirstByte;
	}

	bool getSkipBlockType() const {
		return skipBlockType;
	}

	bool getSkipDelimiter() const {
		return skipDelimiter;
	}

	bool getSkipPmsVersion() const {
		return skipPmsVersion;
	}

	virtual void executePreHandshake(TlsSession & session) override;
	virtual void executePreStep(TlsSession & session) override;
	virtual void executePostStep(TlsSession & session) override;
	virtual void executePostHandshake(TlsSession & session) override;

private:
	bool manipulationInProgress{false};
	const bool skipFirstByte;
	const bool skipBlockType;
	const bool skipDelimiter;
	const bool skipPmsVersion;
};
}

#endif /* MANIPULATION_SKIPRSAESPKCS1V15PADDINGCHECK_H_ */
