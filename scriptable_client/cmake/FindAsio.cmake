#.rst:
# FindAsio
# ------------
#
# Find Asio, a cross-platform C++ library for network and low-level I/O programming.
#
# Imported targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` target:
#
# ``Asio::Asio``
#   The Asio library, if found.
#
# Result variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``Asio_INCLUDE_DIRS``
#   Where to find asio.hpp
# ``Asio_FOUND``
#   If false, do not try to use Asio.
# ``Asio_VERSION_STRING``
#   The version of the Asio library found.

#=============================================================================
# Copyright (C) 2016 Benjamin Eikel <benjamin.eikel@achelos.de>
#
# All rights reserved.
#=============================================================================

find_path(Asio_INCLUDE_DIR
	asio.hpp
)

if(Asio_INCLUDE_DIR)
	set(Asio_INCLUDE_DIRS ${Asio_INCLUDE_DIR})

	if(NOT TARGET Asio::Asio)
		add_library(Asio::Asio INTERFACE IMPORTED)
		list(APPEND Asio_FLAGS ASIO_STANDALONE ASIO_DISABLE_THREADS ASIO_NO_DEPRECATED)
		set(Asio_LIBS "")
		set_target_properties(Asio::Asio PROPERTIES
			INTERFACE_COMPILE_DEFINITIONS "${Asio_FLAGS}")
		set_target_properties(Asio::Asio PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${Asio_INCLUDE_DIRS}")
		set_target_properties(Asio::Asio PROPERTIES
			INTERFACE_LINK_LIBRARIES "${Asio_LIBS}")
	endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Asio DEFAULT_MSG
	Asio_INCLUDE_DIR
)

mark_as_advanced(
	Asio_INCLUDE_DIR
)
