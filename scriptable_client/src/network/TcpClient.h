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
#ifndef NETWORK_TCPCLIENT_H_
#define NETWORK_TCPCLIENT_H_

#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

namespace TlsTestTool {
class AbstractSocketObserver;
/**
 * TCP/IP client socket.
 */
class TcpClient {
public:
	class Data;

private:
	//! Use pimpl idiom.
	std::unique_ptr<Data> impl;

public:
	/**
	 * Construct a non-connected TCP/IP client socket.
	 */
	TcpClient();

	/**
	 * Free the TCP/IP client socket.
	 */
	~TcpClient();

	/**
	 * Connect the socket to the given host and port.
	 *
	 * @param IP address or host name of the host to connect to.
	 * @param port TCP port number of the service to connect to.
	 * @throw std::exception Thrown on failure.
	 */
	void connect(const std::string & host, const std::string & port);

	/**
	 * Close an open connection.
	 *
	 * @throw std::exception Thrown on failure, e.g., no open connection.
	 */
	void close();

	/**
	 * Write a block of characters to the TCP/IP client socket.
	 *
	 * @param data Data block to write.
	 * @return Number of bytes written, if successful. Zero, otherwise.
	 * @throw std::exception Thrown on failure.
	 */
	std::size_t write(const std::vector<char> & data);

	/**
	 * Read a block of characters from the TCP/IP client socket.
	 *
	 * @param length Number of bytes to read. The call will return only successfully, if this number of bytes have been
	 * read.
	 * @return Data block that has been read.
	 * @throw std::exception Thrown on failure.
	 */
	std::vector<char> read(std::size_t length);

	/**
	 * Get the number of bytes that are available for reading.
	 *
	 * @return Number of bytes that can be read.
	 * @throw std::exception Thrown on failure.
	 */
	std::size_t available() const;

	/**
	 * Check, if the connection is closed.
	 *
	 * @return @code true, if the socket is not connected. @code false, if a connection exists.
	 * @throw std::exception Thrown on failure.
	 */
	bool isClosed();

	/**
	 * Check, if the connection is closed.
	 *
	 * @param pollOne @code true, call poll_one(). @code false, call poll() two times.
	 * @return @code true, if the socket is not connected. @code false, if a connection exists.
	 * @throw std::exception Thrown on failure.
	 */
	bool isClosed(bool pollOne);

	/**
	 * Get the IP address of a connected peer.
	 *
	 * @return IP address in dotted decimal format.
	 * @throw std::exception Thrown on failure, e.g., no open connection.
	 */
	std::string getRemoteIpAddress() const;

	/**
	 * Get the TCP port of a connected peer.
	 *
	 * @return TCP port number
	 * @throw std::exception Thrown on failure, e.g., no open connection.
	 */
	uint16_t getRemoteTcpPort() const;

	/**
	 * Access internal implementation data.
	 *
	 * @return Internal implemenation data.
	 */
	Data & getImplementationData();

    /**
     * Register an observer that will be notified on blocks that are written or read.
     * @param observer Observer to register
     */
    void registerObserver(AbstractSocketObserver & observer);
};
}

#endif /* NETWORK_TCPCLIENT_H_ */
