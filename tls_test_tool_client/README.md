# TLS Test Tool


The TLS Test Tool is able to test a huge variety of TLS clients and servers. This README gives an overview over the structure of the TLS Test Tool and its usage. Furthermore, interfaces on different levels as well as the input and output formats for their correct usage are explained. 

As a client, the TLS Test Too establishes a TCP/IP connection and starts a TLS handshake by sending a [ClientHello message](https://datatracker.ietf.org/doc/html/rfc5246#section-7.4.1.2). As server, it binds to a TCP port, waits for an incoming connection and an incoming
ClientHello message and respondeds with a [ServerHello message](https://datatracker.ietf.org/doc/html/rfc5246#section-7.4.1.3). Then, the TLS handshake takes
place. After the handshake, the TLS Test Tool sends a [Closure Alert](https://datatracker.ietf.org/doc/html/rfc5246#section-7.2.1), closes the TCP/IP connection,
and exits. The user can influence this default behavior by using one or more of the provided manipulations.

## Building the Tool

The TLS Test Tool uses CMake as its build system and requires a C++14-compatible compiler. For
building external libraries, Perl and patch are required. For example, on a Debian GNU/Linux system,
the required software can be acquired by installing the build-essential and cmake packages.

To build the tool from the command line, perform the following steps:

```bash
cd tls_test_tool_client
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

After a successful build, the TlsTestTool binary can be found at `build/src/TlsTestTool`. 

## Configuration
The available arguments for configuring the TLS Test Tool are described here. Values that a user
has to provide are denoted in square brackets (e.g., [length] for a value named length).

### Command line arguments
The TLS Test Tool expects at least one argument on the command line.
```bash
--configFile=[configuration file path]
```
Specify the path to a configuration file. When this argument is given multiple times, the
given configuration files are read. Options from configuration files that are given later on the
command line will overwrite options from those given earlier.

Examples:
```bash
TlsTestTool --configFile=config/TestCase27.conf
TlsTestTool --configFile=tlsOptions.conf
```
### Configuration file
The configuration for the TLS Test Tool is given in a configuration file. The configuration file
is a plain text file. Lines that start with the hash sign (#) are treated as comments and ignored.
Arguments are given as name-value pairs separated with the equals sign (=). The following arguments
are known.

#### Input of binary data
Binary data is given in hexadecimal form. The bytes of a byte array have to be encoded separately
and printed separated by a space character. Each byte is represented by two digits from 0-9a-f. For
example, the array of the two bytes 0xc0 0x30 has to be given as c0 30. In the following, the word
HEXSTRING- is used as placeholder for an arbitrary byte array. Please note that an empty byte array is
possible and has to be represented by an empty string.



#### Network options

* mode=[mode]

  (required, with mode either client or server)
Specify the mode for the TLS Test Tool. If mode=client, the TLS Test Tool will run as a TLS
client and connect to a server using TCP/IP. If mode=server, the TLS Test Tool will run as a
TLS server and listen for incoming TCP/IP connections.


* host=[host]

  (required, if mode=client, with string host)
If mode=client, specify a host name or IP address that the TLS Test Tool should connect to.
Ignored, if mode=server.

* port=[port]

  (required, with decimal integer port)
If mode=client, the TCP port of a service to connect to. If mode=server, the TCP port to
bind and listen to on the local host.

* listenTimeout=[timeout]

  (with decimal integer timeout)
If mode=server, the TLS Test Tool will exit if no incoming TCP/IP connection is received
within timeout seconds. If not specified or timeout equals zero, the TLS Test Tool will listen
forever. Ignored, if mode=client.

* waitBeforeClose=[timeout]

  (with decimal integer timeout)
Specify the timeout in milliseconds that the tool waits for incoming data after a run before
closing the TCP/IP connection.

* receiveTimeout=[timeout]

  (with decimal integer timeout)
Specify the timeout in milliseconds that the tool waits for incoming TCP/IP packets during a
receive operation.




#### TLS options
* certificateFile=[path]

  (with path pointing to a PEM- or DER-encoded file)
File containing a X.509 certificate that will be used as server or client certificate, respectively,
depending on the mode.

* privateKeyFile=[path]

  (with path pointing to a PEM- or DER-encoded file)
File containing a private key that matches the certificate’s public key.

* tlsVersion=([major],[minor])

  (with decimal integers major equal to 3 and minor from [1, 3])
If mode=client, send the specified version in ClientHello.client_version. If mode=server, accept
only the specified version and send it in ServerHello.server_version. Use (3,1) for TLS v1.0, (3,2) for TLS v1.1, (3,3) for TLS v1.2. If not specified, all three TLS versions are accepted
by a server and the highest version is sent by a client.

* tlsCipherSuites=([valueUpper],[valueLower])[,([valueUpper],[valueLower])...]

  (with hexadecimal integers valueUpper and valueLower preceded with 0x)
Specify a list of supported TLS cipher suites in decreasing order of preference. If this option is
set, at least one TLS cipher suite has to be given. If mode=client, send the list in ClientHello.cipher_
suites. If mode=server, use this list to find a matching TLS cipher suite to send in Server-
Hello.cipher_suite. The values correspond to the values from the [TLS Cipher Suite Registry](https://datatracker.ietf.org/doc/html/rfc5246#section-7.2.1). For
example, the value (0xC0,0x2C) corresponds to TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384. If not specified, a default list of TLS cipher suites is used.

* tlsSecretFile=[path]

  (with path pointing to an output file)
Append the master_secret in the NSS Key Log Format to a plain text file. This file can be
used by Wireshark to decrypt TLS packets.


#### Other options
* tlsServerSimulation=[predefined]

  (with predefined equal to a key defined below)
If mode=server, mimic the behavior of the configured server in the case of an invalid padding in
the received [RSAES-PKCS1-V1_5-ENCRYPT](https://datatracker.ietf.org/doc/html/rfc3447#section-7.2.1) block or invalid pre-master secret (see ROBOT
attack for more information).
The value of predefined can be one of the keys given in the following table.


Key | Product | CVE | Response correct | Response incorrect | Workflow | Padding check
--- | --- | --- | --- | --- | --- | ---
1 | Cisco | ACE | 2017-17428 | Alert 20 | Alert 47 | CKE, CCS, FIN | Reduced
2 | Facebook v2 | - | Timeout | TCP Finished | CKE | Reduced
3 | F5 v1 | 2017-6168 Timeout | Alert 40 | CKE | Reduced
4 | PAN OS | 2017-17841 Alert 40 | Alert 40 twice | CKE, CCS, FIN | Full
5 | Netscaler GCM | 2017-17382 | Alert 51 | Timeout | CKE, CCS, FIN | Reduced
6 | Delay | - | Alert 40 | Delayed Alert 40 | CKE, CCS, FIN | Full


* tlsServerSimulationDelay=[delay in microseconds]

  If mode=server and tlsServerSimulation=6, the server delays the response by the specified
value when an invalid padding is received. The valid range is 0-1000000.
Please note that this configuration only has an impact of the handshake if the key exchange
method is RSA. Ignored, if mode=client.



#### Procedure manipulations
* manipulateSkipChangeCipherSpec=[ignored]

  (with an arbitrary, possibly empty value ignored)
Skip sending a [ChangeCipherSpec](https://datatracker.ietf.org/doc/html/rfc5246#section-7.1) message and directly send a [Finished](https://datatracker.ietf.org/doc/html/rfc5246#section-7.4.9) message.
* manipulateSkipFinished=[ignored]

  (with an arbitrary, possibly empty value ignored)
If mode=client, skip sending a [Finished](https://datatracker.ietf.org/doc/html/rfc5246#section-7.4.9) message. Ignored, if mode=server.

#### Miscellaneous manipulations
* manipulatePreMasterSecretRandom=[ignored]

  (with an arbitrary, possibly empty value ignored)
If mode=client, replace the field [PreMasterSecret.random](https://datatracker.ietf.org/doc/html/rfc5246#section-7.4.7.1) in a ClientKeyExchange message
with non-zero random bytes. The manipulation is done before performing the encrypting that
results in EncryptedPreMasterSecret. Please note that the structure EncryptedPreMasterSecret
is created only if the key exchange method is RSA. Ignored, if mode=server.
* manipulatePreMasterSecretRandomByte=[index]

  (with positive, decimal integer index) If mode=client, replace the byte with the given index in the field [PreMasterSecret.random](https://datatracker.ietf.org/doc/html/rfc5246#section-7.4.7.1) in a ClientKeyExchange message with a zero byte. Since the length of the field is 46 bytes, the maximum allowed index is 45. The manipulation is done before performing the encrypting that
results in EncryptedPreMasterSecret. Please note that the structure EncryptedPreMasterSecret
is created only if the key exchange method is RSA. Ignored, if mode=server.
* manipulatePreMasterSecretVersion=([major],[minor])

  (with hexadecimal integers major and minor preceded with 0x)
If mode=client, replace the field [PreMasterSecret.client_version](https://datatracker.ietf.org/doc/html/rfc5246#section-7.4.7.1) in a ClientKeyExchange
message with the two bytes given in major and minor. The manipulation is done before
performing the encrypting that results in EncryptedPreMasterSecret. Please note that the
structure EncryptedPreMasterSecret is created only if the key exchange method is RSA. Ignored,
if mode=server.
* manipulateRsaesPkcs1V15EncryptPadding=[firstByte],[blockType],[padding]
  (with 3 hexadecimal integer firstByte, blockType and padding preceded with 0x)
If mode=client, overwrite the first byte with the value of firstByte, the block type byte with
the value of blockType and the value of the byte between PS and M with the value of padding
in [RSAES-PKCS1-V1_5-ENCRYPT](https://datatracker.ietf.org/doc/html/rfc3447#section-7.2.1) when using RSA to create the [EncryptedPreMasterSecret](https://datatracker.ietf.org/doc/html/rfc5246#section-7.4.7.1)
in a [ClientKeyExchange](https://datatracker.ietf.org/doc/html/rfc5246#section-7.4.7) message. Ignored, if mode=server.
* manipulateSkipRsaesPkcs1V15PaddingCheck=[firstByte],[blockType],[delimiter],[pmsVersion]

  (with boolean values firstByte, blockType, delimiter, pmsVersion either true or false)
If mode=server, skip the check of the first byte if firstByte is true, the block type if blockType
is true, the delimiter if delimiter is true and the version in the pre-master secret if pmsVersion
is true in the [RSAES-PKCS1-V1_5-ENCRYPT](https://datatracker.ietf.org/doc/html/rfc3447#section-7.2.1). Ignored, if mode=client

#### Examples

Example for running as a TLS client:

```bash
# Connect to a web server
mode=client
host=tls-check.de
port=443
```
Example for running as a TLS server:
```bash
# Run locally on the LDAPS port
mode=server
port=636
certificateFile=server/certificate.pem
privateKeyFile=server/private/key.pem
```

## License
[Licensed under the EUPL, Version 1.2](https://joinup.ec.europa.eu/collection/eupl/eupl-text-eupl-12)

## Third party Licenses

The licenses of 3rd party software used within TLS Test Tool are listed below.

### Mbed TLS version 2.26.0
Website: [https://tls.mbed.org](https://tls.mbed.org)  
Source: [https://github.com/ARMmbed/mbedtls/archive/refs/tags/v2.26.0.tar.gz](https://github.com/ARMmbed/mbedtls/archive/refs/tags/v2.26.0.tar.gz)  
License: [Apache License Version 2.0](http://www.apache.org/licenses/LICENSE-2.0)


### Zlib version 1.2.11
Website: [https://www.zlib.net](https://www.zlib.net)  
Source: [http://download.sourceforge.net/project/libpng/zlib/1.2.11/zlib-1.2.11.tar.gz](http://download.sourceforge.net/project/libpng/zlib/1.2.11/zlib-1.2.11.tar.gz)  
License: [zlib License](https://zlib.net/zlib_license.html)


