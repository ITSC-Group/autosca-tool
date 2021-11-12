cmake_minimum_required(VERSION 3.4.0)

add_library(TlsTestToolManipulation STATIC
	ManipulatePreMasterSecretRandom.cpp
	ManipulatePreMasterSecretRandom.h
	ManipulatePreMasterSecretRandomByte.cpp
	ManipulatePreMasterSecretRandomByte.h
	ManipulatePreMasterSecretVersion.cpp
	ManipulatePreMasterSecretVersion.h
	ManipulateRsaesPkcs1V15EncryptPadding.cpp
	ManipulateRsaesPkcs1V15EncryptPadding.h
	Manipulation.cpp
	Manipulation.h
	ManipulationsParser.cpp
	ManipulationsParser.h
	SkipChangeCipherSpec.cpp
	SkipChangeCipherSpec.h
	SkipFinished.cpp
	SkipFinished.h
	SkipRsaesPkcs1V15PaddingCheck.cpp
	SkipRsaesPkcs1V15PaddingCheck.h
)
target_include_directories(TlsTestToolManipulation PRIVATE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
)
target_link_libraries(TlsTestToolManipulation PRIVATE
	Tooling
)
