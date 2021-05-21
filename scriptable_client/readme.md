# AutoSCA TLS Test Tool

## Sentinel dongle driver installation

1. Disconnect your dongle (if any) from the computer.
2. Open a terminal window and navigate to the directory `tls-test-tool/dongle driver`.
3. Enter the command: `sudo ./dinst .` 
   (Note that the final "." in the command is required for the file location.)
4. Reconnect the dongle. 


## TLS Test Tool execution

1. Open a terminal window and navigate to the `tls-test-tool` directory.
2. Run TLS Test Tool with command: `./TlsTestTool --configFile=./config/sampleMinimum.conf`
   (If you want to write the default output to a file use: `./TlsTestTool --configFile=./config/sampleMinimum.conf > handshake.log`)

Supplied sample configurations:

 - ./config/sampleEcdhe.conf          // Only TLS_ECDHE_* cipher suites
 - ./config/sampleCertAuth.conf       // Certificate validation via given CA certificate file
 - ./config/sampleDestroyMac.conf     // Manipulate MAC in the finish message


## Python client

There is a work-in-progress python client that runs the TLS Test Tool in several processes in parallel.

1. Open a terminal window and navigate to the `tls-test-tool` directory.
2. Install the virtual environment python package with `pip3 install --user pipenv`
3. Reload the profile file with `bash --login` or open a new terminal
4. Install the necessary python packages inside the virtual environment with `pipenv install`
5. Execute the script in the virtual python environment with `pipenv run python3 client.py`

## TLS Test Tool log messages

These are the possible messages that can be used for validation.

```
public enum TlsTestToolResource {
	/** After_ClientHello.*/
	After_ClientHello("after ClientHello"),
	/** After_Handshake.*/
	After_Handshake("afterHandshake"),
	/** After_ServerHello.*/
	After_ServerHello("after ServerHello"),
	/** Before_Handshake.*/
	Before_Handshake("beforeHandshake"),
	/** Before_ServerHello.*/
	Before_ServerHello("before ServerHello"),
	/** ClientHello_valid.*/
	ClientHello_valid("Valid ClientHello message received."),
	/** ClientHello_version.*/
	ClientHello_version("ClientHello.client_version"),
	/**  for TLS1.3 mapped to the same keyword as the TLS1.2 ClientHello.client_version.*/
	ClientHello_legacy_version("ClientHello.client_version"),
	/** ClientHello_bad.*/
	ClientHello_bad("Bad ClientHello message received."),
	/** ClientHello_random.*/
	ClientHello_random("ClientHello.random"),
	/** ClientHello_extensions.*/
	ClientHello_extensions("ClientHello.extensions"),
	/** ClientHello_cipher_suites.*/
	ClientHello_cipher_suites("ClientHello.cipher_suites"),
	/** ClientHello_compression_methods.*/
	ClientHello_compression_methods("ClientHello.compression_methods"),
	/** CertificateRequest_valid.*/
	CertificateRequest_valid("Valid CertificateRequest message received."),
	/** ClientHello_session_id.*/
	ClientHello_session_id("ClientHello.session_id"),
	/** CertificateRequest_bad.*/
	CertificateRequest_bad("Bad CertificateRequest message received."),
	/** CertificateVerify_valid.*/
	CertificateVerify_valid("Valid CertificateVerify message received."),
	/** CertificateVerify_bad.*/
	CertificateVerify_bad("Bad CertificateVerify message received."),
	/** CertificateVerify_algorithm.*/
	CertificateVerify_algorithm("CertificateVerify.algorithm"),
	/** Certificate_valid.*/
	Certificate_valid("Valid Certificate message received."),
	/** Certificate_bad.*/
	Certificate_bad("Bad Certificate message received."),
	/** Certificate_list_size.*/
	Certificate_list_size("Certificate.certificate_list.size"),
	   /** Certificate_extensions_list.*/
    Certificate_extensions("Certificate.certificate_list[0].extensions_list"),
	/** Certificate_transmitted.*/
	Certificate_transmitted("Certificate message transmitted."),
	/** ChangeCipherSpec_valid.*/
	ChangeCipherSpec_valid("Valid ChangeCipherSpec message received."),
	/** ChangeCipherSpec_bad.*/
	ChangeCipherSpec_bad("Bad ChangeCipherSpec message received."),
	/** ClientKeyExchange_message_transmitted.*/
	ClientKeyExchange_message_transmitted("ClientKeyExchange message transmitted."),
	/** ClientKeyExchange_valid.*/
	ClientKeyExchange_valid("Valid ClientKeyExchange message received."),
	/** ClientKeyExchange_bad.*/
	ClientKeyExchange_bad("Bad ClientKeyExchange message received."),
	/** ClientKeyExchange_exchange_keys_pre_master_secret.*/
	ClientKeyExchange_exchange_keys_pre_master_secret(
			"ClientKeyExchange.exchange_keys.pre_master_secret"),
	/** Compression_null.*/
	Compression_null("00"),
	/** Compression_DEFLATE.*/
	Compression_DEFLATE("01"),
	/** Compression_LZS.*/
	Compression_LZS(Integer.toString(64, 16)),
	/** Finished_valid.*/
	Finished_valid("Valid Finished message received."),
	/** Finished_bad.*/
	Finished_bad("Bad Finished message received."),
	/** ServerHelloDone_transmitted.*/
	ServerHelloDone_transmitted("ServerHelloDone message transmitted."),
	/** Handshake_aborted.*/
	Handshake_aborted("Handshake aborted."),	
	/** HelloRetryRequest_valid.*/
	HelloRetryRequest_valid("Valid HelloRetryRequest message received."),
	/** HelloRetryRequest_bad.*/
	HelloRetryRequest_bad("Bad HelloRetryRequest message received."),
	/** HelloRetryRequest_server_version.*/
	HelloRetryRequest_server_version("HelloRetryRequest.server_version"),
	/** HelloRetryRequest_legacy_version.*/
	HelloRetryRequest_legacy_version("HelloRetryRequest.server_version"), 
	/** HelloRetryRequest_random.*/
	HelloRetryRequest_random("HelloRetryRequest.random"),
	/** HelloRetryRequest_extensions.*/
	HelloRetryRequest_extensions("HelloRetryRequest.extensions"),
	/** HelloRetryRequest_session_id.*/
	HelloRetryRequest_session_id("HelloRetryRequest.session_id"),
	/** HelloRetryRequest_cipher_suite.*/
	HelloRetryRequest_cipher_suite("HelloRetryRequest.cipher_suite"),
	/** HelloRetryRequest_compression_method.*/
	HelloRetryRequest_compression_method("HelloRetryRequest.compression_method"),
	/** ServerHelloDone_valid.*/
	ServerHelloDone_valid("Valid ServerHelloDone message received."),
	/** ServerHelloDone_bad.*/
	ServerHelloDone_bad("Bad ServerHelloDone message received."),
	/** ServerHello_valid.*/
	ServerHello_valid("Valid ServerHello message received."),
	/** ServerHello_bad.*/
	ServerHello_bad("Bad ServertHello message received."),
	/** ServerHello_server_version.*/
	ServerHello_server_version("ServerHello.server_version"),
	/** for TLS1.3 mapped to the same keyword as the TLS1.2 ServerHello.server_version. */
	ServerHello_legacy_version("ServerHello.server_version"),
	/** ServerHello_random.*/
	ServerHello_random("ServerHello.random"),
	/** ServerHello_extensions.*/
	ServerHello_extensions("ServerHello.extensions"),
	/** ServerHello_session_id.*/
	ServerHello_session_id("ServerHello.session_id"),
	/** ServerHello_cipher_suite.*/
	ServerHello_cipher_suite("ServerHello.cipher_suite"),
	/** ServerHello_compression_method.*/
	ServerHello_compression_method("ServerHello.compression_method"),
	/** ServerKeyExchange_valid.*/
	ServerKeyExchange_valid("Valid ServerKeyExchange message received."),
	/** ServerKeyExchange_bad.*/
	ServerKeyExchange_bad("Bad ServerKeyExchange message received."),
	/** ServerKeyExchange_params_dh_p.*/
	ServerKeyExchange_params_dh_p("ServerKeyExchange.params.dh_p"),
	/** ServerKeyExchange_params_dh_g.*/
	ServerKeyExchange_params_dh_g("ServerKeyExchange.params.dh_g"),
	/** ServerKeyExchange_params_dh_Ys.*/
	ServerKeyExchange_params_dh_Ys("ServerKeyExchange.params.dh_Ys"),
	/** ServerKeyExchange_params_namedcurve.*/
	ServerKeyExchange_params_namedcurve("ServerKeyExchange.params.curve_params.namedcurve"),
	/** ServerKeyExchange_params_public.*/
	ServerKeyExchange_params_public("ServerKeyExchange.params.public"),
	/** ServerKeyExchange_signed_params_algorithm_hash.*/
	ServerKeyExchange_signed_params_algorithm_hash("ServerKeyExchange.signed_params.algorithm.hash"),
	/** ServerKeyExchange_signed_params_algorithm_signature.*/
	ServerKeyExchange_signed_params_algorithm_signature("ServerKeyExchange.signed_params.algorithm.signature"),
	/** ServerKeyExchange_signed_params_signature.*/
	ServerKeyExchange_signed_params_signature("ServerKeyExchange.signed_params.signature"),
	/** ServerKeyExchange_signed_params_md5_hash.*/
	ServerKeyExchange_signed_params_md5_hash("ServerKeyExchange.signed_params.md5_hash"),
	/** ServerKeyExchange_signed_params_sha_hash.*/
	ServerKeyExchange_signed_params_sha_hash("ServerKeyExchange.signed_params.sha_hash"),
	/** FinishedGenericBlockCipherIV.*/
	FinishedGenericBlockCipherIV("Finished.GenericBlockCipher.IV"),
	/** TCP_IP_Conn_to.*/
	TCP_IP_Conn_to("TCP/IP connection to"),
	/** TCP_IP_Conn_to_established.*/
	TCP_IP_Conn_to_established("TCP/IP connection to (.*) established."),
	/** Waiting_TCP_IP_conn_port.*/
	Waiting_TCP_IP_conn_port("Waiting for TCP/IP connection on port"),
	/** TCP_IP_conn_from.*/
	TCP_IP_conn_from("TCP/IP connection from"),
	/** TCP_IP_Conn_closed.*/
	TCP_IP_Conn_closed("TCP/IP connection is closed."),
	/** Alert_message_received.*/
	Alert_message_received("Alert message received."),
	/** Alert_level.*/
	Alert_level("Alert.level"),
	/** Alert_description.*/
	Alert_description("Alert.description"),
	/** Log_Dump_Description.*/
	Log_Dump_Description("The following log consist of TLS Test Tool log output, and TShark log output, if enabled."
					+ " PLEASE NOTE: In case of a test case working with iterations, only the last iteration"
					+ " will be displayed. The log file on disk contains the whole log."),
	/** Handshake_failed.*/
	Handshake_failed("TLS handshake failed"),
	/** Handshake_failed_no_ciphersuites_in_common.*/
	Handshake_failed_no_ciphersuites_in_common("The server has no ciphersuites in common with the client"),
	/** Supported_signature_algorithms.*/
	Supported_signature_algorithms("CertificateRequest.supported_signature_algorithms"),
	/** Read_size.*/
	Read_size("Read.size"),
	/** Read_timestamp.*/
	Read_timestamp("Read.timestamp"),
	/** Write_size.*/
	Write_size("Write.size"),
	/** Write_timestamp.*/
	Write_timestamp("Write.timestamp"),
	/** Message_received_suffix.*/
	Message_received_suffix("message received."),
	/** Message_transmitted_suffix.*/
	Message_transmitted_suffix("message transmitted."),
	/** NewSessionTicket_ticket.*/
	NewSessionTicket_ticket("NewSessionTicket.ticket"),
	/** Heartbeat_message_size.*/
	Heartbeat_message_size("Heartbeat data size including padding"),
	/** Heartbeat_type.*/
	Heartbeat_type("Heartbeat.type"),
	/** Heartbeat_payload_length.*/
	Heartbeat_payload_length("Heartbeat.payload_length"),
	/** Heartbeat_payload_data.*/
	Heartbeat_payload_data("Heartbeat.payload_data");
}
```
