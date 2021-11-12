cmake_minimum_required(VERSION 3.4.0)

message(STATUS "CMAKE_BUILD_TYPE  ${CMAKE_BUILD_TYPE}")

set(FLAGS_WARN "-pedantic-errors -Werror -Wall -Wextra")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAGS_WARN}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS_WARN}")
	
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wformat-overflow=2 -Wformat-truncation=2")
set(CMAKE_C_FLAGS_RELEASE     "-O2")
set(CMAKE_C_FLAGS_DEBUG       "-O0 -g3")
set(CMAKE_C_FLAGS_COVERAGE    "-O0 -g3 --coverage")
set(CMAKE_C_FLAGS_ASAN        "-fsanitize=address -fno-common -fsanitize=undefined -fno-sanitize-recover=all -O3")
set(CMAKE_C_FLAGS_ASANDBG     "-fsanitize=address -fno-common -fsanitize=undefined -fno-sanitize-recover=all -O1 -g3 -fno-omit-frame-pointer -fno-optimize-sibling-calls")
set(CMAKE_C_FLAGS_CHECK       "-Os")
set(CMAKE_C_FLAGS_CHECKFULL   "${CMAKE_C_FLAGS_CHECK} -Wcast-qual")

add_subdirectory(configuration)
add_subdirectory(manipulation)
add_subdirectory(network)
add_subdirectory(tls)

add_library(TlsTestToolLib INTERFACE)
target_include_directories(TlsTestToolLib INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(TlsTestToolLib INTERFACE TlsTestToolConfiguration)
target_link_libraries(TlsTestToolLib INTERFACE TlsTestToolManipulation)
target_link_libraries(TlsTestToolLib INTERFACE TlsTestToolNetwork)
target_link_libraries(TlsTestToolLib INTERFACE TlsTestToolTls)
target_link_libraries(TlsTestToolLib INTERFACE Tooling)

add_executable(TlsTestTool
	TlsTestTool.cpp
)
target_compile_definitions(TlsTestTool PRIVATE TLS_TEST_TOOL_VERSION="${PROJECT_VERSION}")
target_link_libraries(TlsTestTool TlsTestToolLib)


install(TARGETS TlsTestTool
    RUNTIME DESTINATION .
)

