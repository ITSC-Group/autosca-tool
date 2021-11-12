cmake_minimum_required(VERSION 3.4.0)

include(ExternalProject)

set (EP_ASIO "EP_ASIO")
set (EP_ZLIB "EP_ZLIB")
set (EP_MBEDTLS "EP_MBEDTLS")

set (EXTRA_CMAKE_ARGS)

ExternalProject_Add(${EP_ASIO}
    URL https://nav.dl.sourceforge.net/project/asio/asio/1.18.2%20%28Stable%29/asio-1.18.2.tar.gz
    URL_HASH SHA256=3d76241275e1c3a833850f798cc54184a0ac28254d8c32aaed1ae3a8630fa353
    BUILD_ALWAYS ON
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory
                    <SOURCE_DIR>/include ${EP_INSTALL_DIR}/include/
)
list (APPEND EXTRA_CMAKE_ARGS
    -DAsio_INCLUDE_DIR=${EP_INSTALL_DIR}/include
)

list (APPEND EXTRA_CMAKE_ARGS
    -DEP_INCLUDEDIR=${EP_INSTALL_DIR}/include
    -DEP_LIBDIR=${EP_INSTALL_DIR}/lib   
)

ExternalProject_Add(${EP_ZLIB}
	URL http://download.sourceforge.net/project/libpng/zlib/1.2.11/zlib-1.2.11.tar.gz
	URL_HASH SHA256=c3e5e9fdd5004dcb542feda5ee4f0ff0744628baf8ed2dd5d66f8ca1197cb1a1
	PATCH_COMMAND patch -p0 < ${CMAKE_CURRENT_LIST_DIR}/zlib-RenameLibraryOutputName.patch && patch -p0 < ${CMAKE_CURRENT_LIST_DIR}/zlib-DoNotInstallDynamicLibrary.patch
	BUILD_ALWAYS ON
	INSTALL_DIR ${EP_INSTALL_DIR}
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
)
  
ExternalProject_Add(${EP_MBEDTLS}
    DEPENDS ${EP_ZLIB}
    URL https://github.com/ARMmbed/mbedtls/archive/refs/tags/v2.26.0.tar.gz
    URL_HASH SHA256=37949e823c7e1f6695fc56858578df355da0770c284b1c1304cfc8b396d539cd
    PATCH_COMMAND patch -p1 < ${CMAKE_CURRENT_LIST_DIR}/mbedTLS-EnableZlibSupport.patch && patch -p1 < ${CMAKE_CURRENT_LIST_DIR}/mbedTLS-OverwritePreMasterSecretClientVersion.patch && patch -p1 < ${CMAKE_CURRENT_LIST_DIR}/mbedTLS-OverwritePreMasterSecretRandom.patch && patch -p1 < ${CMAKE_CURRENT_LIST_DIR}/mbedTLS-WrongPaddingRsaesPkcs1V15Encrypt.patch && patch -p1 < ${CMAKE_CURRENT_LIST_DIR}/mbedTLS-ManipulateServerSimulation.patch && patch -p1 < ${CMAKE_CURRENT_LIST_DIR}/mbedTLS-CompilationFix.patch && patch -p1 < ${CMAKE_CURRENT_LIST_DIR}/mbedTLS-Debug.patch && patch -p1 < ${CMAKE_CURRENT_LIST_DIR}/mbedTLS-SkipPaddingCheckRsaesPkcs1V15Encrypt.patch
    BUILD_ALWAYS ON
    INSTALL_DIR ${EP_INSTALL_DIR}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE} -DENABLE_ZLIB_SUPPORT:BOOL=ON -DENABLE_TESTING:BOOL=OFF -DENABLE_PROGRAMS:BOOL=OFF -DZLIB_LIBRARY=<INSTALL_DIR>/lib -DZLIB_INCLUDE_DIR=<INSTALL_DIR>/include
    TEST_BEFORE_INSTALL 0
)

set (DEPENDENCIES)
list (APPEND DEPENDENCIES ${EP_ASIO} ${EP_ZLIB} ${EP_MBEDTLS})

ExternalProject_Add (EP_TLSTESTTOOL
  DEPENDS ${DEPENDENCIES}
  SOURCE_DIR ${PROJECT_SOURCE_DIR}
  CMAKE_ARGS -DUSE_SUPERBUILD=OFF ${EXTRA_CMAKE_ARGS}
  BUILD_ALWAYS ON
  INSTALL_COMMAND ""
  BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}
)
