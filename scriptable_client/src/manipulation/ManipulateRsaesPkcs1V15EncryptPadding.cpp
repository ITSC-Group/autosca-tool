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
#include "ManipulateRsaesPkcs1V15EncryptPadding.h"
#include "strings/StringHelper.h"
#include "tls/TlsSession.h"

namespace TlsTestTool {
void ManipulateRsaesPkcs1V15EncryptPadding::executePreHandshake(TlsSession & /*session*/) {
}

void ManipulateRsaesPkcs1V15EncryptPadding::executePreStep(TlsSession & session) {
	if (session.isClient() && (TlsHandshakeState::CLIENT_KEY_EXCHANGE == session.getState())) {
		log(__FILE__, __LINE__, "Change the first byte in RSAES-PKCS1-V1_5-ENCRYPT to "
					+ Tooling::StringHelper::formatInt("0x%02x", firstByte) + '.');
		log(__FILE__, __LINE__, "Change the block type byte in RSAES-PKCS1-V1_5-ENCRYPT to "
					+ Tooling::StringHelper::formatInt("0x%02x", blockType) + '.');
		log(__FILE__, __LINE__, "Change the byte between PS and M in RSAES-PKCS1-V1_5-ENCRYPT to "
					+ Tooling::StringHelper::formatInt("0x%02x", padding) + '.');
		manipulationInProgress = true;
		session.overwriteRsaesPkcs1V15EncryptPadding(firstByte, blockType, padding);
	}
}

void ManipulateRsaesPkcs1V15EncryptPadding::executePostStep(TlsSession & session) {
	if (manipulationInProgress) {
		// Reset manipulation.
		session.restoreRsaesPkcs1V15EncryptPadding();
		manipulationInProgress = false;
	}
}

void ManipulateRsaesPkcs1V15EncryptPadding::executePostHandshake(TlsSession & /*session*/) {
}
}
