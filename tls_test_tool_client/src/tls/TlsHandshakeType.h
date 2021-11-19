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
#ifndef TLS_TLSHANDSHAKETYPE_H_
#define TLS_TLSHANDSHAKETYPE_H_

#include "tls/TlsNumbers.h"

namespace TlsTestTool {
/**
 * Types of TLS handshake messages.
 *
 * @see RFC 5246, Section 7.4
 */
enum class TlsHandshakeType : TlsUint8 {
	HELLO_REQUEST = 0,
	CLIENT_HELLO = 1,
	SERVER_HELLO = 2,
	CERTIFICATE = 11,
	SERVER_KEY_EXCHANGE = 12,
	CERTIFICATE_REQUEST = 13,
	SERVER_HELLO_DONE = 14,
	CERTIFICATE_VERIFY = 15,
	CLIENT_KEY_EXCHANGE = 16,
	FINISHED = 20
};
}

#endif /* TLS_TLSHANDSHAKETYPE_H_ */
