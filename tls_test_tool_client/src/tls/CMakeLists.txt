cmake_minimum_required(VERSION 3.4.0)

add_subdirectory(mbedtls)

add_library(TlsTestToolTls STATIC
	TlsCipherSuite.h
	TlsContentType.h
	TlsHandshakeHeader.h
	TlsHandshakeState.h
	TlsHandshakeType.h
	TlsHashAlgorithm.h
	TlsMessage.h
	TlsNumbers.h
	TlsPlaintextHeader.h
	TlsSession.cpp
	TlsSession.h
	TlsSessionFactory.cpp
	TlsSessionFactory.h
	TlsSignatureAlgorithm.h
	TlsSignatureAndHashAlgorithm.h
	TlsVersion.h
)
target_include_directories(TlsTestToolTls PRIVATE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
)
target_link_libraries(TlsTestToolTls PRIVATE
	Tooling
	TlsTestToolTlsMbedTls
)
