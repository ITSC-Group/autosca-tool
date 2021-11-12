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
#include "TimestampObserver.h"
#include "TcpClient.h"
#include "logging/Logger.h"
#include "private/TcpClientData.h"
#include <chrono>

#ifdef USE_SO_TIMESTAMPING
#include "strings/StringHelper.h"
#include <array>
#include <cerrno>
#include <cstring>
#include <ctime>
#include <linux/errqueue.h>
#include <linux/net_tstamp.h>
#include <sys/socket.h>
#include <sys/types.h>
#endif /* USE_SO_TIMESTAMPING */

namespace TlsTestTool {

#ifdef USE_SO_TIMESTAMPING
static inline std::string toString(const timespec & ts) {
	return std::to_string(ts.tv_sec) + Tooling::StringHelper::formatInt("%09ld", ts.tv_nsec);
}
#endif /* USE_SO_TIMESTAMPING */

class TimestampObserver::Data {
public:
	TcpClient & socket;
	Tooling::Logger & logger;
	bool useSoTimestamping{false};

	Data(TcpClient & _socket, Tooling::Logger & _logger) : socket(_socket), logger(_logger) {
	}

	void log(const std::string & file, const int line, const std::string & message) {
		logger.log(Tooling::LogLevel::HIGH, "Network", file, line, message);
	}

#ifdef USE_SO_TIMESTAMPING
	int getSocketFd() const {
		return socket.getImplementationData().getSocket().native_handle();
	}

	void logIf(const std::string & file, const int line, ifreq * interface, const std::string & message) {
		log(file, line, std::string{interface->ifr_name} + ": " + message);
	}

	static const timespec * getSwTimestamp(msghdr & msgHdr) {
		for (cmsghdr * cMsgHdr = CMSG_FIRSTHDR(&msgHdr); nullptr != cMsgHdr; cMsgHdr = CMSG_NXTHDR(&msgHdr, cMsgHdr)) {
			if ((SOL_SOCKET == cMsgHdr->cmsg_level) && (SCM_TIMESTAMPING == cMsgHdr->cmsg_type)) {
				const auto timestamping = reinterpret_cast<const scm_timestamping *>(CMSG_DATA(cMsgHdr));
				return timestamping->ts;
			}
		}
		return nullptr;
	}
#endif /* USE_SO_TIMESTAMPING */
};

TimestampObserver::TimestampObserver(TcpClient & socket, Tooling::Logger & logger)
		: impl(std::make_unique<Data>(socket, logger)) {
#ifdef USE_SO_TIMESTAMPING
	const auto fd = impl->getSocketFd();
	// Enable software time stamping
	// SOF_TIMESTAMPING_TX_HARDWARE is not used here, because RX time stamps are not supported for TCP.
	// SOF_TIMESTAMPING_SOFTWARE time stamps are comparable to user space time stamps.
	const int flags = SOF_TIMESTAMPING_TX_SOFTWARE | SOF_TIMESTAMPING_SOFTWARE | SOF_TIMESTAMPING_OPT_TSONLY;
	if (setsockopt(fd, SOL_SOCKET, SO_TIMESTAMPING, &flags, sizeof(flags)) < 0) {
		impl->log(__FILE__, __LINE__, std::string{"setsockopt with SO_TIMESTAMPING failed: "} + std::strerror(errno));
	}
	// Check socket options
	int actualFlags;
	socklen_t len = sizeof(actualFlags);
	if (getsockopt(fd, SOL_SOCKET, SO_TIMESTAMPING, &actualFlags, &len) < 0) {
		impl->log(__FILE__, __LINE__, std::string{"getsockopt with SO_TIMESTAMPING failed: "} + std::strerror(errno));
	} else if (flags != actualFlags) {
		impl->log(__FILE__, __LINE__,
				  "Expected SO_TIMESTAMPING flags equal " + std::to_string(flags)
						  + ", actual SO_TIMESTAMPING flags equal " + std::to_string(actualFlags) + '.');
	} else {
		impl->useSoTimestamping = true;
	}
#endif /* USE_SO_TIMESTAMPING */
	if (impl->useSoTimestamping) {
		impl->log(__FILE__, __LINE__,
				  "Create timestamps with SO_TIMESTAMPING (TX) and std::chrono::high_resolution_clock (RX).");
	} else {
		impl->log(__FILE__, __LINE__, "Create timestamps with std::chrono::high_resolution_clock.");
	}
}

TimestampObserver::~TimestampObserver() = default;

void TimestampObserver::onBlockWritten(std::size_t length) noexcept {
	const auto currentTime = std::chrono::high_resolution_clock::now();
	impl->log(__FILE__, __LINE__, "Write.size=" + std::to_string(length));
#ifdef USE_SO_TIMESTAMPING
	// Read ancillary data from socket's error queue
	std::array<char, 512> controlBuffer;

	msghdr msgHdr;
	std::memset(&msgHdr, 0, sizeof(msgHdr));
	msgHdr.msg_control = controlBuffer.data();
	msgHdr.msg_controllen = controlBuffer.size();

	static const std::chrono::seconds receiveTimeout(3);
	const auto timeout = currentTime + receiveTimeout;

	ssize_t result;
	while (true) {
		result = recvmsg(impl->getSocketFd(), &msgHdr, MSG_ERRQUEUE);
		if (0 > result) {
			const auto error = errno;
			if ((EAGAIN != error) && (EWOULDBLOCK != error)) {
				impl->log(__FILE__, __LINE__, std::string{"recvmsg failed: "} + std::strerror(error));
				break;
			}
			if (std::chrono::high_resolution_clock::now() > timeout) {
				impl->log(__FILE__, __LINE__,
						  "SO_TIMESTAMPING (TX) failed. Falling back to std::chrono::high_resolution_clock.");
				impl->useSoTimestamping = false;
				break;
			}
		} else {
			break;
		}
	}

	const auto swTimestamp = impl->getSwTimestamp(msgHdr);
	if (nullptr != swTimestamp) {
		impl->log(__FILE__, __LINE__, "Write.timestamp=" + toString(*swTimestamp));
		return;
	}
#endif /* USE_SO_TIMESTAMPING */
	if (!impl->useSoTimestamping) {
		const auto timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime.time_since_epoch());
		impl->log(__FILE__, __LINE__, "Write.timestamp=" + std::to_string(timestamp.count()));
	}
}

void TimestampObserver::onBlockRead(std::size_t length) noexcept {
	// Receive timestamps (e.g., with SOF_TIMESTAMPING_RX_HARDWARE) do not work for TCP.
	const auto currentTime = std::chrono::high_resolution_clock::now();
	impl->log(__FILE__, __LINE__, "Read.size=" + std::to_string(length));
	const auto timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime.time_since_epoch());
	impl->log(__FILE__, __LINE__, "Read.timestamp=" + std::to_string(timestamp.count()));
}
}
