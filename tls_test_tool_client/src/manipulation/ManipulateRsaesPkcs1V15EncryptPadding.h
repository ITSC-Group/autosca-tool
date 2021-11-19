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
#ifndef MANIPULATION_MANIPULATERSAESPKCS1V15ENCRYPTPADDING_H_
#define MANIPULATION_MANIPULATERSAESPKCS1V15ENCRYPTPADDING_H_

#include "Manipulation.h"
#include "tls/TlsHandshakeState.h"
#include <cstdint>

namespace TlsTestTool {
/**
 * Overwrite the first byte, the block type byte and the byte between PS and M in RSAES-PKCS1-V1_5-ENCRYPT when sending a ClientKeyExchange message.
 * Valid parameters are: 0x00, 0x02, 0x01
 * Encryption-block: firstByte = 00 || blockType = 02 || PS || padding = 00 || D .
 * @see https://tools.ietf.org/html/rfc3447#section-7.2.1
 */
class ManipulateRsaesPkcs1V15EncryptPadding : public Manipulation {
public:
	/**
	 * Create a manipulation.
	 *
	 * @param firstByte First byte value that will be used. 0x00 is the correct value.
	 * @param blockTypeByte Block type value that will be used. 0x02 is the correct value (encryption only).
	 * @param paddingByte Padding value that will be used. 0x00 is the correct value.
	 */
	ManipulateRsaesPkcs1V15EncryptPadding(const uint8_t firstByteByte,
										  const uint8_t blockTypeByte,
										  const uint8_t paddingByte) : Manipulation(), firstByte(firstByteByte), blockType(blockTypeByte), padding(paddingByte) {
	}

	uint8_t getFirstByte() const {
		return firstByte;
	}

	uint8_t getBlockType() const {
		return blockType;
	}

	uint8_t getPadding() const {
		return padding;
	}

	virtual void executePreHandshake(TlsSession & session) override;
	virtual void executePreStep(TlsSession & session) override;
	virtual void executePostStep(TlsSession & session) override;
	virtual void executePostHandshake(TlsSession & session) override;

private:
	bool manipulationInProgress{false};
	const uint8_t firstByte;
	const uint8_t blockType;
	const uint8_t padding;
};
}

#endif /* MANIPULATION_MANIPULATERSAESPKCS1V15ENCRYPTPADDING_H_ */
