cmake_minimum_required (VERSION 3.8 FATAL_ERROR)

if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.17)
	cmake_policy (SET CMP0100 NEW)
endif ()

project (topas)

enable_testing()

set (TOPAS_VERSION_MAJOR 4)
set (TOPAS_VERSION_MINOR 0)
set (TOPAS_VERSION_PATCH 0)

# Print compiler information
message (STATUS "CMAKE_CXX_COMPILER_ID: " ${CMAKE_CXX_COMPILER_ID})
message (STATUS "CMAKE_CXX_COMPILER_VERSION: " ${CMAKE_CXX_COMPILER_VERSION})

# Set cmake module Path and load modules
set (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules ${CMAKE_MODULE_PATH})
include (TOPAS_Options)
include (TOPAS_FindPackages)
include (TOPAS_BuildSettings)

# Options summary
message (STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message (STATUS "TOPAS_MT: ${TOPAS_MT}")
message (STATUS "TOPAS_WITH_STATIC_GEANT4: ${TOPAS_WITH_STATIC_GEANT4}")
message (STATUS "TOPAS_COVERAGE: ${TOPAS_COVERAGE}")
message (STATUS "TOPAS_INSTALL_EXAMPLES: ${TOPAS_INSTALL_EXAMPLES}")
message (STATUS "TOPAS-VERSION: ${TOPAS_VERSION_MAJOR}.${TOPAS_VERSION_MINOR}.${TOPAS_VERSION_PATCH}")

#
# Set up the file tree

configure_file (
	"${CMAKE_CURRENT_SOURCE_DIR}/TsTopasConfig.hh.in"
	"${CMAKE_CURRENT_BINARY_DIR}/TsTopasConfig.hh")

include (CMakeHandleExtensions.cmake)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/extensions
	${PROJECT_SOURCE_DIR}/filtering
	${PROJECT_SOURCE_DIR}/chemistry
	${PROJECT_SOURCE_DIR}/geometry
	${PROJECT_SOURCE_DIR}/graphics
	${PROJECT_SOURCE_DIR}/io
	${PROJECT_SOURCE_DIR}/main
	${PROJECT_SOURCE_DIR}/material
	${PROJECT_SOURCE_DIR}/outcome
	${PROJECT_SOURCE_DIR}/parameter
	${PROJECT_SOURCE_DIR}/physics
	${PROJECT_SOURCE_DIR}/primary
	${PROJECT_SOURCE_DIR}/scoring
	${PROJECT_SOURCE_DIR}/sequence
	${PROJECT_SOURCE_DIR}/variance
)

# Invoke cmake on these sub-directories, too
add_subdirectory (${CMAKE_CURRENT_BINARY_DIR}/extensions extensions)
add_subdirectory (filtering)
add_subdirectory (chemistry)
add_subdirectory (geometry)
add_subdirectory (graphics)
add_subdirectory (io)
add_subdirectory (main)
add_subdirectory (material)
add_subdirectory (outcome)
add_subdirectory (parameter)
add_subdirectory (physics)
add_subdirectory (primary)
add_subdirectory (scoring)
add_subdirectory (sequence)
add_subdirectory (variance)

# set RPATH on UNIX systems, taking special care of Apple OSs [must be done before add_executable]
if (APPLE)
	set (CMAKE_INSTALL_RPATH "@executable_path/../Frameworks")
elseif (UNIX)
	set (CMAKE_INSTALL_RPATH "\$ORIGIN/../lib")
else ()
	message (AUTHOR_WARNING "Target platform is not UNIXoid, not setting RPATH automatically. Good luck!")
endif ()

# main() wrapper calling TsMain
add_executable (topas topas.cc)

# Link executable against these
target_link_libraries (topas
	main
	parameter
	chemistry
	geometry
	extensions
	graphics
	material
	physics
	variance
	filtering
	scoring
	outcome
	io
	sequence
	primary
	gdcmMSFF
	${Geant4_LIBRARIES}
)

#
# Target: install

# executable
install (TARGETS topas RUNTIME DESTINATION bin)

# LICENSE/README, headers, libraries, etc.
# LICENSE & README
install (FILES
	forenduser/LICENSE.txt
	forenduser/rundemos.csh
	DESTINATION .)
	
# files for end user compiling
install (FILES
	topas.cc
	CMakeHandleExtensions.cmake
	DESTINATION .)
install (FILES
	extensions/TsExtensionManager.cc.in
	extensions/TsExtensionManager.hh.in
	extensions/CMakeLists.txt.in
	DESTINATION extensions)

# TOPAS headers and libraries
include (TOPAS_InstallLibs)

# External (Geant4, GDCM, ...) libraries
include (TOPAS_InstallExternalLibs)

# Examples
if (NOT DEFINED TOPAS_INSTALL_EXAMPLES OR ${TOPAS_INSTALL_EXAMPLES})
	include (TOPAS_InstallExamples)
endif ()

#
# Target: package

set (CPACK_RESOURCE_FILE_LICENSE ${PROJECT_SOURCE_DIR}/forenduser/LICENSE.txt)
set (CPACK_SET_DESTDIR TRUE)
set (CPACK_INSTALL_PREFIX /topas)
set (CPACK_GENERATOR "TGZ")
set (CPACK_PACKAGE_VERSION_MAJOR ${TOPAS_VERSION_MAJOR})
set (CPACK_PACKAGE_VERSION_MINOR ${TOPAS_VERSION_MINOR})
set (CPACK_PACKAGE_VERSION_PATCH ${TOPAS_VERSION_PATCH})
include (CPack)

#
# CI / Testing

include (TOPAS_Tests)
