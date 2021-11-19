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
#ifndef TLS_TLSSESSION_H_
#define TLS_TLSSESSION_H_

#include "tls/TlsCipherSuite.h"
#include "tls/TlsHandshakeState.h"
#include "tls/TlsHashAlgorithm.h"
#include "tls/TlsSignatureAndHashAlgorithm.h"
#include "tls/TlsVersion.h"
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <memory>
#include <utility>
#include <vector>

namespace Tooling {
class Logger;
}
namespace TlsTestTool {
class TcpClient;
class TcpServer;
class TlsSession;
using TlsCallbackFunction = std::function<void(TlsSession &)>;
/**
 * Encapsulation of structures of a TLS session.
 */
class TlsSession {
public:
	/**
	 * Construct a TLS client session.
	 * @param tcpClient Connected TCP/IP client that will be used to send and receive data.
	 */
	TlsSession(TcpClient & tcpClient);

	/**
	 * Construct a TLS server session.
	 * @param tcpClient TCP/IP server that will be used to send and receive data.
	 */
	TlsSession(TcpServer & tcpServer);

	/**
	 * Free the TLS session's data.
	 */
	virtual ~TlsSession();

	/**
	 * Access the TCP/IP socket associated with the TLS session.
	 * @return TCP/IP socket that is be used to send and receive data
	 */
	TcpClient & getSocket();

	/**
	 * Configure the certificate and the private key to be used by this TLS session. In case this TLS session is used as
	 * a server, the given certificate should be a server certificate. In case this TLS session is used as a client, the
	 * given certificate should be a client certificate.
	 * @param certificate The certificate is read from the stream as PEM- or DER-encoded data.
	 * @param privateKey The private key is read from the stream as PEM- or DER-encoded data.
	 */
	virtual void setCertificate(std::istream & certificate, std::istream & privateKey) = 0;

	/**
	 * Perform a TLS handshake. The underlying TCP/IP connection has to exist already.
	 */
	virtual void performHandshake() = 0;

	/**
	 * Perform a stop of the TLS handshake. The underlying TCP/IP connection has to exist already.
	 */
	virtual void performHandshakeStep() = 0;

	/**
	 * Send that given data as application data over the existing TLS session.
	 * @param data Data to send
	 */
	virtual void sendApplicationData(const std::vector<uint8_t> & data) = 0;

	/**
	 * Receive  application data over the existing TLS session.
	 * @return Data that has been received
	 */
	virtual std::vector<uint8_t> receiveApplicationData() = 0;

	/**
	 * Close the TLS session by sending a close_notify alert.
	 */
	virtual void close(const uint32_t closeTimeout) = 0;

	/**
	 * Return the current state of the TLS session.
	 * @return Current TLS handshake state
	 */
	virtual TlsHandshakeState getState() const = 0;

	/**
	 * Manipulate the current state of the TLS session.
	 * @param manipulatedState New TLS handshake state
	 */
	virtual void setState(TlsHandshakeState manipulatedState) = 0;

	/**
	 * Return the current TLS version, if negotiated.
	 * @return TLS version, if negotiated. (0, 0), otherwise.
	 */
	virtual TlsVersion getVersion() const = 0;

	/**
	 * Restrict the TLS version that will be negotiated.
	 * @param TLS version to negotiate.
	 */
	virtual void setVersion(const TlsVersion & version) = 0;

	/**
	 * Set the list of supported TLS cipher suites.
	 * @param List of TLS cipher suites
	 */
	virtual void setCipherSuites(const std::vector<TlsCipherSuite> & cipherSuites) = 0;

	/**
	 * Check whether this TLS session is a TLS client or a TLS server session.
	 * @return @code true, if this is a TLS client session. @code false, otherwise.
	 */
	bool isClient() const;

	/**
	 * Overwrite the field PreMasterSecret.client_version in a ClientKeyExchange message before encryption.
	 * @param newValue Value used for overwriting the field
	 */
	virtual void overwritePreMasterSecretVersion(const TlsVersion & newValue) = 0;

	/**
	 * Overwrite the field PreMasterSecret.random in a ClientKeyExchange message before encryption.
     * The fiel PreMasterSecret.random is filled with new non-zero random bytes.
     * The client continues to use the original random bytes.
	 */
	virtual void overwritePreMasterSecretRandom() = 0;

	/**
	 * Overwrite the byte at the given index in the field PreMasterSecret.random with zero.
     * The client continues to use the original random bytes.
     * @param index Index of the byte which will be set to zero.
	 */
	virtual void overwritePreMasterSecretRandomByte(uint16_t index) = 0;

	/**
	 * Manipulate the firstbyte, the block type and the padding field between PS and M in RSAES-PKCS1-V1_5-ENCRYPT.
	 * @param firstByte First byte value to use. The correct value is 0x00.
	 * @param blockType Block type value to use. The correct value is 0x02 (encryption mode only).
	 * @param padding Padding value to use. The correct value is 0x00.
	 * @see restoreRsaesPkcs1V15EncryptPadding
	 */
	virtual void overwriteRsaesPkcs1V15EncryptPadding(const uint8_t firstByte,
													  const uint8_t blockType,
													  const uint8_t padding) = 0;

	/**
	 * Restore the default behaviour and remove the manipulation of the padding in RSAES-PKCS1-V1_5-ENCRYPT.
	 * @see overwriteRsaesPkcs1V15EncryptPadding
	 */
	virtual void restoreRsaesPkcs1V15EncryptPadding() = 0;

	/**
	 * Manipulate the check behavior of the padding in RSAES-PKCS1-V1_5-ENCRYPT.
	 * @param skipFirstByte Skip first byte check.
	 * @param skipBlockType Skip block type check.
	 * @param skipDelimiter Skip delimiter check.
	 * @param skipPmsVersion Skip pre-master secret version check.
	 * @see restoreRsaesPkcs1V15EncryptPaddingCheck
	 */
	virtual void skipRsaesPkcs1V15EncryptPaddingCheck(const bool skipFirstByte,
													  const bool skipBlockType,
													  const bool skipDelimiter,
													  const bool skipPmsVersion) = 0;

	/**
	 * Restore the default check behavior of the padding in RSAES-PKCS1-V1_5-ENCRYPT.
	 * @see skipRsaesPkcs1V15EncryptPaddingCheck
	 */
	virtual void restoreRsaesPkcs1V15EncryptPaddingCheck() = 0;

	/**
	 * Attach a logger that will be used for log output.
	 * @param logger Log that will receive log entries.
	 */
	virtual void setLogger(Tooling::Logger & logger);

	/**
	 * Set an output stream that the master_secret in the NSS Key Log Format will be written to.
	 * @param output Output stream.
	 * @see https://developer.mozilla.org/en-US/docs/Mozilla/Projects/NSS/Key_Log_Format
	 */
	void setSecretOutput(std::unique_ptr<std::ostream> && output);

	/**
	 * Attach a callback function that is called before a TLS handshake step is executed. For example, it can be used to
	 * perform manipulations on this TLS session.
	 * @param callback Callback function that receives a TLS session as parameter.
	 */
	void registerPreStepCallback(TlsCallbackFunction && callback);

	/**
	 * Attach a callback function that is called after a TLS handshake step has been executed. For example, it can be
	 * used to extract information from this TLS session.
	 * @param callback Callback function that receives a TLS session as parameter.
	 */
	void registerPostStepCallback(TlsCallbackFunction && callback);

	/**
	 * Set the time which specifies how many seconds should be waited for an alert from peer.
	 * @param timeout The waiting time in seconds.
	 */
	virtual void setWaitForAlertSeconds(const uint32_t timeout) = 0;

	/**
	 * Set the time which specifies how many seconds should be waited for receiving TCP packets.
	 * @param timeout The waiting time in seconds.
	 */
	virtual void setTcpReceiveTimeoutSeconds(const uint32_t timeout) = 0;

	/**
	 * Set the ID of the server to be simulated.
	 * @param id The ID of the simulated server (0 means no simulation).
	 */
	virtual void setServerSimulation(const uint16_t id) = 0;

    /**
     * Set the delay of the server to be simulated in microseconds
     * @param delayInMicroseconds The delay of the simulated server in microseconds.
     */
    virtual void setServerSimulationDelay(const useconds_t delayInMicroseconds) = 0;

protected:
	/**
	 * Write a log message.
	 * @param file File name of the message's origin.
	 * @param line Line number of the message's origin.
	 * @param message Message that should be written to the log.
	 */
	void log(const std::string & file, const int line, const std::string & message);

	/**
	 * Call attached callback functions before a TLS handshake step is executed.
	 */
	void onPreStep();

	/**
	 * Call attached callback functions after a TLS handshake step has been executed.
	 */
	void onPostStep();

	/**
	 * Check whether secret information should be collected.
	 * @return @code true, if secrets should be collected. @code false, otherwise.
	 * @see setSecretOutput
	 * @see provideSecrectInformation
	 */
	bool isSecrectInformationCollected();

	/**
	 * Provide the master secrect of a TLS session.
	 * @param clientRandom The 32 bytes random value of the ClientHello message. It is used to identify the TLS session.
	 * @param masterSecret The 48 bytes of the TLS session's master secret.
	 * @see setSecretOutput
	 * @see isSecrectInformationCollected
	 */
	void provideSecrectInformation(const std::vector<uint8_t> & clientRandom,
								   const std::vector<uint8_t> & masterSecret);

private:
	class Data;
	//! Use pimpl idiom.
	std::unique_ptr<Data> impl;
};
}

#endif /* TLS_TLSSESSION_H_ */
