#
# MUST BE INCLUDED -NOT BEFORE- TOPAS_Options
# SHALL BE INCLUDED AFTER INCLUDING EXTERNAL PACKAGES
#

#
# Define flags in addition to the Geant4 CXX flags

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang") # using GCC || CLANG
	set (CMAKE_CXX_FLAGS_WARNINGS "-Wall -Wextra -Wpedantic")
	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		set (CMAKE_CXX_FLAGS_WARNINGS "${CMAKE_CXX_FLAGS_WARNINGS} -Wmaybe-uninitialized")
	endif ()
	if (APPLE)
		set (CMAKE_CXX_FLAGS_BASE "")
	else ()
		set (CMAKE_CXX_FLAGS_BASE "-Wl,--disable-new-dtags")
	endif ()
	set (CMAKE_CXX_FLAGS_DEBUG "-Og -g")
	set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
	set (CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -fno-trapping-math -ftree-vectorize -fno-math-errno")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel") # Intel ICC
	message (WARNING "We do not support the use of the Intel compiler collection. Use this at your own risk.")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") # Microsoft VisualC(++)
	message (WARNING "We do not support the use of MSVC. Use this at your own risk.")
	set (CMAKE_CXX_FLAGS_WARNINGS "/W4")
	set (CMAKE_CXX_FLAGS_BASE "/MP")
else ()
	message (WARNING "You are using an unknown compiler. Use this at your own risk.")
endif()

# Set the C++ Standard
set (CMAKE_CXX_STANDARD 17)

#
# Add the CXX Flags

message (STATUS "Prepending the following flags to CXX_FLAGS = ${CMAKE_CXX_FLAGS}")
message (STATUS "(BASICS)   ${CMAKE_CXX_FLAGS_BASE}")
message (STATUS "(WARNINGS) ${CMAKE_CXX_FLAGS_WARNINGS}")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_BASE} ${CMAKE_CXX_FLAGS_WARNINGS} ${CMAKE_CXX_FLAGS_DEBUG} ${CMAKE_CXX_FLAGS}")
	message (STATUS "(DEBUG)    ${CMAKE_CXX_FLAGS_DEBUG}")
elseif (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_BASE} ${CMAKE_CXX_FLAGS_WARNINGS} ${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${CMAKE_CXX_FLAGS}")
	message (STATUS "(RELW/DEB) ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
else () # assume Release
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_BASE} ${CMAKE_CXX_FLAGS_WARNINGS} ${CMAKE_CXX_FLAGS_RELEASE} ${CMAKE_CXX_FLAGS}")
	message (STATUS "(RELEASE)  ${CMAKE_CXX_FLAGS_RELEASE}")
endif ()

#
# Add coverage for code analysis to compiler flags

if (TOPAS_COVERAGE)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 --coverage")
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 --coverage")
	if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
		message (WARNING "Using TOPAS_COVERAGE=${TOPAS_COVERAGE} with non-Debug build type. This can cause undocumented behavior.")
	endif()
endif()
