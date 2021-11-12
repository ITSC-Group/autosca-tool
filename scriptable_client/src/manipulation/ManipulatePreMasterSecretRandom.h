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
#ifndef MANIPULATION_MANIPULATEPREMASTERSECRETRANDOM_H_
#define MANIPULATION_MANIPULATEPREMASTERSECRETRANDOM_H_

#include "Manipulation.h"
#include "tls/TlsVersion.h"

namespace TlsTestTool {
/**
 * Change the field PreMasterSecret.random before encrypting it to create EncryptedPreMasterSecret.
 * The fiel PreMasterSecret.random is filled with new non-zero random bytes. 
 * The client continues to use the original random bytes.
 */
class ManipulatePreMasterSecretRandom : public Manipulation {
public:
	/**
	 * Create a manipulation.
	 */
	ManipulatePreMasterSecretRandom() : Manipulation() {
	}

	virtual void executePreHandshake(TlsSession & session) override;
	virtual void executePreStep(TlsSession & session) override;
	virtual void executePostStep(TlsSession & session) override;
	virtual void executePostHandshake(TlsSession & session) override;
};
}

#endif /* MANIPULATION_MANIPULATEPREMASTERSECRETRANDOM_H_ */
