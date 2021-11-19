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
#ifndef NETWORK_TCPCLIENTDATA_H_
#define NETWORK_TCPCLIENTDATA_H_

#include "asio.hpp"
#include "network/AbstractSocketObserver.h"
#include <functional>
#include <vector>

namespace TlsTestTool {
/**
 * Private class containing implementation details.
 */
class TcpClient::Data {
private:
	asio::io_context io_context;
	asio::ip::tcp::socket socket;
	bool connectionClosedByError;
	std::vector<std::reference_wrapper<AbstractSocketObserver>> observers;

public:
	Data() : io_context(), socket(io_context), connectionClosedByError(false), observers() {
	}

	asio::io_context & getContext() {
		return io_context;
	}

	asio::ip::tcp::socket & getSocket() {
		return socket;
	}

	const asio::ip::tcp::socket & getSocket() const {
		return socket;
	}

	bool isConnectionClosedByError() const {
		return connectionClosedByError;
	}

	void closeConnectionByError() {
		connectionClosedByError = true;
	}

	void registerObserver(AbstractSocketObserver & observer) {
		observers.emplace_back(std::ref(observer));
	}

	void notifyWrite(std::size_t length) {
		for (auto & observer : observers) {
			observer.get().onBlockWritten(length);
		}
	}

	void notifyRead(std::size_t length) {
		for (auto & observer : observers) {
			observer.get().onBlockRead(length);
		}
	}
};
}

#endif /* NETWORK_TCPCLIENTDATA_H_ */
