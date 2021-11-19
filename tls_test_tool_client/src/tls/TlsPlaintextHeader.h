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
#ifndef TLS_TLSPLAINTEXTHEADER_H_
#define TLS_TLSPLAINTEXTHEADER_H_

#include "tls/TlsContentType.h"
#include "tls/TlsNumbers.h"
#include "tls/TlsVersion.h"

namespace TlsTestTool {
/**
 * Type for accessing a TLSPlaintext record's header elements.
 *
 * @see RFC 5246, Section 6.2.1
 */
class TlsPlaintextHeader {
public:
	TlsContentType type;
	TlsVersion version;
	TlsUint16 length;
};
}

#endif /* TLS_TLSPLAINTEXTHEADER_H_ */
