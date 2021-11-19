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
#include "SkipChangeCipherSpec.h"
#include "tls/TlsSession.h"

namespace TlsTestTool {
void SkipChangeCipherSpec::executePreHandshake(TlsSession & /*session*/) {
}

void SkipChangeCipherSpec::executePreStep(TlsSession & session) {
    if (session.isClient()) {
        // Client
        if (TlsHandshakeState::CLIENT_CHANGE_CIPHER_SPEC == session.getState()) {
            log(__FILE__, __LINE__, "Skip sending ChangeCipherSpec message.");
            session.setState(TlsHandshakeState::CLIENT_FINISHED);
        }
    } else {
        // Server
        if (TlsHandshakeState::SERVER_CHANGE_CIPHER_SPEC == session.getState()) {
            log(__FILE__, __LINE__, "Skip sending ChangeCipherSpec message.");
            session.setState(TlsHandshakeState::SERVER_FINISHED);
        }
    }
}

void SkipChangeCipherSpec::executePostStep(TlsSession & /*session*/) {
}

void SkipChangeCipherSpec::executePostHandshake(TlsSession & /*session*/) {
}
}
