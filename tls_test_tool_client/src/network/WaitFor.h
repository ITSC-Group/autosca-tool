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
#ifndef NETWORK_WAITFOR_H_
#define NETWORK_WAITFOR_H_

#include <cstddef>
#include <cstdint>

namespace TlsTestTool {
class TcpServer;
/**
 * Helper to wait for specific conditions on a TCP/IP server.
 */
class WaitFor {
public:
	/**
	 * Create a helper object.
	 * @param tcpServer Specific server instance
	 * @param timeoutSeconds Timeout for wait operations
	 */
	WaitFor(TcpServer & tcpServer, const uint32_t timeoutSeconds = 3);

	/**
	 * Wait for an incoming client connection on the TCP/IP server.
	 * @return @c true, if client has connected. @c false, if a timeout occurred.
	 */
	bool clientConnection() const;

	/**
	 * Wait until a given amount of data is available for reading from a connected client.
	 * @param expectedLength Number of bytes that should be available for reading
	 * @return @c true, if the requested amount of data is available for reading. @c false, if a timeout occurred.
	 */
	bool clientData(const std::size_t expectedLength) const;

private:
	TcpServer & server;
	const uint32_t timeout;
};
}

#endif /* NETWORK_WAITFOR_H_ */
