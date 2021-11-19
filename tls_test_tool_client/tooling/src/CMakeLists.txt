#
# This file is part of the Tooling project.
# Copyright (C) 2016 Benjamin Eikel <benjamin.eikel@achelos.de>
#
# All rights reserved.
#
cmake_minimum_required(VERSION 3.4.0)

add_library(Tooling STATIC
	configuration/CommandLineParser.cpp
	configuration/ConfigurationFileParser.cpp
	logging/Logger.cpp
	strings/HexStringHelper.cpp
)
target_include_directories(Tooling PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

