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
#ifndef NETWORK_TCPSERVER_H_
#define NETWORK_TCPSERVER_H_

#include <cstdint>
#include <memory>

namespace TlsTestTool {
class TcpClient;
/**
 * TCP/IP server socket working with a single client connection.
 */
class TcpServer {
public:
	/**
	 * Construct a non-connected TCP/IP server socket.
	 */
	TcpServer();

	/**
	 * Free the TCP/IP server socket.
	 */
	~TcpServer();

	/**
	 * Bind the TCP/IP server socket and listen for incoming connections.
	 *
	 * @param port TCP port number to listen to.
	 * @throw std::exception Thrown on failure.
	 */
	void listen(uint16_t port);

	/**
	 * Close the TCP/IP server socket and stop listen for incoming connections.
	 */
	void close();

	/**
	 * Perform pending tasks on the TCP/IP server socket. This function has to be called regularly from the event loop.
	 */
	void work();

	/**
	 * Get a TCP/IP client that is possibly connected.
	 *
	 * @return TCP/IP client socket
	 */
	TcpClient & getClient();

private:
	class Data;
	//! Use pimpl idiom.
	std::unique_ptr<Data> impl;
};
}

#endif /* NETWORK_TCPSERVER_H_ */
