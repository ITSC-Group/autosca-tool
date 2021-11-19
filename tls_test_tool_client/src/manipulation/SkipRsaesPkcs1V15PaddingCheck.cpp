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
#include "SkipRsaesPkcs1V15PaddingCheck.h"
#include "strings/StringHelper.h"
#include "tls/TlsSession.h"

namespace TlsTestTool {
void SkipRsaesPkcs1V15PaddingCheck::executePreHandshake(TlsSession & /*session*/) {
}

void SkipRsaesPkcs1V15PaddingCheck::executePreStep(TlsSession & session) {
	if (!session.isClient() && (TlsHandshakeState::CLIENT_KEY_EXCHANGE == session.getState())) {
		log(__FILE__, __LINE__, "Skip the first byte check in RSAES-PKCS1-V1_5-ENCRYPT: "
					+ Tooling::StringHelper::formatInt("%d", skipFirstByte) + '.');
		log(__FILE__, __LINE__, "Skip the block type check in RSAES-PKCS1-V1_5-ENCRYPT: "
					+ Tooling::StringHelper::formatInt("%d", skipBlockType) + '.');
		log(__FILE__, __LINE__, "Skip the delimiter check in RSAES-PKCS1-V1_5-ENCRYPT: "
					+ Tooling::StringHelper::formatInt("%d", skipDelimiter) + '.');
		log(__FILE__, __LINE__, "Skip the pre-master secret version check in RSAES-PKCS1-V1_5-ENCRYPT: "
					+ Tooling::StringHelper::formatInt("%d", skipPmsVersion) + '.');
		manipulationInProgress = true;
		session.skipRsaesPkcs1V15EncryptPaddingCheck(skipFirstByte, skipBlockType, skipDelimiter, skipPmsVersion);
	}
}

void SkipRsaesPkcs1V15PaddingCheck::executePostStep(TlsSession & session) {
	if (manipulationInProgress) {
		// Reset manipulation.
		session.restoreRsaesPkcs1V15EncryptPaddingCheck();
		manipulationInProgress = false;
	}
}

void SkipRsaesPkcs1V15PaddingCheck::executePostHandshake(TlsSession & /*session*/) {
}
}
