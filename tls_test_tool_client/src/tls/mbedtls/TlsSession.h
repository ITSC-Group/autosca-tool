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
#ifndef TLS_MBEDTLS_TLSSESSION_H_
#define TLS_MBEDTLS_TLSSESSION_H_

#include "tls/TlsSession.h"
#include <memory>

namespace TlsTestTool {
namespace MbedTls {
/**
 * Implementation of TlsSession using mbed TLS.
 */
class TlsSession : public TlsTestTool::TlsSession {
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
	virtual ~TlsSession() override;

	/**
	 * Configure the certificate and the private key to be used by this TLS session. In case this TLS session is used as
	 * a server, the given certificate should be a server certificate. In case this TLS session is used as a client, the
	 * given certificate should be a client certificate.
	 * @param certificate The certificate is read from the stream as PEM- or DER-encoded data.
	 * @param privateKey The private key is read from the stream as PEM- or DER-encoded data.
	 */
	virtual void setCertificate(std::istream & certificate, std::istream & privateKey) override;

	/**
	 * Perform a TLS handshake. The underlying TCP/IP connection has to exist already.
	 */
	virtual void performHandshake() override;

	/**
	 * Perform a stop of the TLS handshake. The underlying TCP/IP connection has to exist already.
	 */
	virtual void performHandshakeStep() override;

	/**
	 * Send that given data as application data over the existing TLS session.
	 * @param data Data to send
	 */
	virtual void sendApplicationData(const std::vector<uint8_t> & data) override;

	/**
	 * Receive  application data over the existing TLS session.
	 * @return Data that has been received
	 */
	virtual std::vector<uint8_t> receiveApplicationData() override;

	/**
	 * Close the TLS session by sending a close_notify alert.
	 */
	virtual void close(const uint32_t closeTimeout) override;

	/**
	 * Return the current state of the TLS session.
	 * @return Current TLS handshake state
	 */
	virtual TlsHandshakeState getState() const override;

	/**
	 * Manipulate the current state of the TLS session.
	 * @param manipulatedState New TLS handshake state
	 */
	void setState(TlsHandshakeState manipulatedState) override;

	/**
	 * Return the current TLS version, if negotiated.
	 * @return TLS version, if negotiated. (0, 0), otherwise.
	 */
	virtual TlsVersion getVersion() const override;

	/**
	 * Restrict the TLS version that will be negotiated.
	 * @param TLS version to negotiate.
	 */
	void setVersion(const TlsVersion & version) override;

	/**
	 * Set the list of supported TLS cipher suites.
	 * @param List of TLS cipher suites
	 */
	virtual void setCipherSuites(const std::vector<TlsCipherSuite> & cipherSuites) override;

	/**
	 * Overwrite the field PreMasterSecret.client_version in a ClientKeyExchange message before encryption.
	 * @param newValue Value used for overwriting the field
	 */
	virtual void overwritePreMasterSecretVersion(const TlsVersion & newValue) override;

	/**
	 * Overwrite the field PreMasterSecret.random in a ClientKeyExchange message before encryption.
     * The fiel PreMasterSecret.random is filled with new non-zero random bytes.
     * The client continues to use the original random bytes.
	 */
	virtual void overwritePreMasterSecretRandom() override;

	/**
	 * Overwrite the byte at the given index in the field PreMasterSecret.random with zero.
     * The client continues to use the original random bytes.
     * @param index Index of the byte which will be set to zero.
	 */
	virtual void overwritePreMasterSecretRandomByte(uint16_t index) override;

	/**
	 * Manipulate the firstbyte, the block type and the padding field between PS and M in RSAES-PKCS1-V1_5-ENCRYPT.
	 * @param firstByte First byte value to use. The correct value is 0x00.
	 * @param blockType Block type value to use. The correct value is 0x02 (encryption mode only).
	 * @param padding Padding value to use. The correct value is 0x00.
	 * @see restoreRsaesPkcs1V15EncryptPadding
	 */
	virtual void overwriteRsaesPkcs1V15EncryptPadding(const uint8_t firstByte,
													  const uint8_t blockType,
													  const uint8_t padding) override;

	/**
	 * Restore the default behaviour and remove the manipulation of the padding in RSAES-PKCS1-V1_5-ENCRYPT.
	 * @see overwriteRsaesPkcs1V15EncryptPadding
	 */
	virtual void restoreRsaesPkcs1V15EncryptPadding() override;

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
													  const bool skipPmsVersion) override;

	/**
	 * Restore the default check behavior of the padding in RSAES-PKCS1-V1_5-ENCRYPT.
	 * @see skipRsaesPkcs1V15EncryptPaddingCheck
	 */
	virtual void restoreRsaesPkcs1V15EncryptPaddingCheck() override;

	/**
	 * Attach a logger that will be used for log output.
	 * @param logger Log that will receive log entries.
	 */
	virtual void setLogger(Tooling::Logger & logger) override;

	/**
	 * Set the time which specifies how many seconds should be waited for an alert from peer.
	 * @param timeout The waiting time in seconds.
	 */
	virtual void setWaitForAlertSeconds(const uint32_t timeout) override;

	/**
	 * Set the time which specifies how many seconds should be waited for receiving TCP packets.
	 * @param timeout The waiting time in seconds.
	 */
	virtual void setTcpReceiveTimeoutSeconds(const uint32_t timeout) override;

	/**
	 * Set the ID of the server to be simulated.
	 * @param id The ID of the simulated server (0 means no simulation).
	 */
	virtual void setServerSimulation(const uint16_t id) override;

    /**
     * Set the delay of the server to be simulated in microseconds
     * @param delayInMicroseconds The delay of the simulated server in microseconds.
     */
    virtual void setServerSimulationDelay(const unsigned int delayInMicroseconds) override;

private:
	class Data;
	//! Use pimpl idiom.
	std::unique_ptr<Data> impl;
};
}
}

#endif /* TLS_MBEDTLS_TLSSESSION_H_ */
