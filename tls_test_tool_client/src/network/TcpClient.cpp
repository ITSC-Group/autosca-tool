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
#include "TcpClient.h"
#include "asio.hpp"
#include "private/TcpClientData.h"
#include <vector>

namespace TlsTestTool {

TcpClient::TcpClient() : impl(std::make_unique<Data>()) {
}

TcpClient::~TcpClient() = default;

void TcpClient::connect(const std::string & host, const std::string & port) {
	asio::ip::tcp::resolver resolver(impl->getContext());
	asio::connect(impl->getSocket(), resolver.resolve(host, port));
	impl->getSocket().set_option(asio::ip::tcp::no_delay(true));
}

void TcpClient::close() {
	impl->getSocket().shutdown(asio::ip::tcp::socket::shutdown_both);
	impl->getSocket().close();
}

std::size_t TcpClient::write(const std::vector<char> & data) {
	try {
        const auto numBytesWritten = asio::write(impl->getSocket(), asio::buffer(data));
        impl->notifyWrite(numBytesWritten);
        return numBytesWritten;
	} catch (const asio::system_error & e) {
		if ((asio::error::connection_aborted == e.code()) || (asio::error::connection_reset == e.code())) {
			impl->closeConnectionByError();
		}
		throw e;
	}
}

std::vector<char> TcpClient::read(std::size_t length) {
	try {
		std::vector<char> buffer(length);
        asio::read(impl->getSocket(), asio::buffer(buffer));
        impl->notifyRead(buffer.size());
        return buffer;
	} catch (const asio::system_error & e) {
		if ((asio::error::connection_aborted == e.code()) || (asio::error::connection_reset == e.code())) {
			impl->closeConnectionByError();
		}
		throw e;
	}
}

std::size_t TcpClient::available() const {
	return impl->getSocket().available();
}

bool TcpClient::isClosed(bool pollOnce) {
	if (impl->isConnectionClosedByError()) {
		return true;
	}
	if (!impl->getSocket().is_open()) {
		return true;
	}
	{
		// No connection, if the remote endpoint cannot be accessed.
		asio::error_code ec;
		impl->getSocket().remote_endpoint(ec);
		if (ec) {
			return true;
		}
	}
	// Check, if the socket is marked readable, which it is, when it is closed (similar to select).
	bool isReadable = false;
	bool isAborted = false;
	bool isReset = false;
	impl->getSocket().async_read_some(asio::null_buffers(), [&](const asio::error_code & error, std::size_t) {
		if (!error) {
			isReadable = true;
		}
		if (asio::error::connection_aborted == error) {
			isAborted = true;
		}
		if (asio::error::connection_reset == error) {
			isReset = true;
		}
	});

	// The original code was replaced because of issues on the Linux operating system
	// (see https://web.achelos.com/jira/browse/TUEVTLSIKE-1629).
	if (pollOnce) {
		impl->getContext().poll_one();
	} else {
		impl->getContext().poll();
		impl->getContext().poll();
	}

	impl->getSocket().cancel();
	// Check, if the number of readable bytes is zero (similar to ioctl with FIONREAD).
	const bool nothingToRead = (0 == available());
	return (isReadable && nothingToRead) || isAborted || isReset;
}

bool TcpClient::isClosed() {
	return isClosed(false);
}

std::string TcpClient::getRemoteIpAddress() const {
	return impl->getSocket().remote_endpoint().address().to_string();
}

uint16_t TcpClient::getRemoteTcpPort() const {
	return impl->getSocket().remote_endpoint().port();
}

TcpClient::Data & TcpClient::getImplementationData() {
	return *impl;
}

void TcpClient::registerObserver(AbstractSocketObserver & observer) {
    impl->registerObserver(observer);
}
}
