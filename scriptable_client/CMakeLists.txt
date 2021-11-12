cmake_minimum_required(VERSION 3.4.0)

set(EP_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/EP_BASE")

option (USE_SUPERBUILD "Whether or not a superbuild should be invoked" ON)

if (USE_SUPERBUILD)
  project (SUPERBUILD NONE)
  include (3rdparty/CMakeLists.txt)
  return()
else()
  project(TlsTestTool VERSION 1.0.0 LANGUAGES C CXX)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

add_compile_options(-Wall -Wextra -Werror)

add_subdirectory(tooling)
add_subdirectory(data)
add_subdirectory(src)

set(CPACK_GENERATOR "ZIP")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(CPACK_ZIP_USE_DISPLAY_NAME_IN_FILENAME ON)
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}")
set(CPACK_COMPONENT_SOURCE_DISPLAY_NAME "Source")
set(CPACK_COMPONENT_UNSPECIFIED_DISPLAY_NAME "${CMAKE_SYSTEM_NAME}")
set(CPACK_PACKAGE_VENDOR "achelos GmbH")
set(CPACK_PACKAGE_VERSION_MAJOR ${TlsTestTool_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${TlsTestTool_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${TlsTestTool_VERSION_PATCH})
include(CPack)
