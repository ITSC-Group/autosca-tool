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
#include "WaitFor.h"
#include "network/TcpClient.h"
#include "network/TcpServer.h"
#include <chrono>
#include <functional>

namespace TlsTestTool {
static bool waitFor(TcpServer & tcpServer, const uint32_t timeoutSeconds, const std::function<bool()> & condition) {
	const std::chrono::seconds timeout(timeoutSeconds);
	auto start = std::chrono::steady_clock::now();
	while ((std::chrono::steady_clock::now() - start) < timeout) {
		tcpServer.work();
		if (condition()) {
			return true;
		}
	}
	return false;
}

WaitFor::WaitFor(TcpServer & tcpServer, const uint32_t timeoutSeconds) : server(tcpServer), timeout(timeoutSeconds) {
}

bool WaitFor::clientConnection() const {
	return waitFor(server, timeout, [this] { return !server.getClient().isClosed(); });
}

bool WaitFor::clientData(const std::size_t expectedLength) const {
	return waitFor(server, timeout,
				   [this, expectedLength] { return expectedLength <= server.getClient().available(); });
}
}
