#
# CMake Options

set (CMAKE_AUTOMOC ON)

if (NOT CMAKE_BUILD_TYPE OR 
	NOT CMAKE_BUILD_TYPE MATCHES "Debug|RelWithDebInfo|Release")
	set (CMAKE_BUILD_TYPE "Release")
	message (WARNING "Setting CMAKE_BUILD_TYPE=Release")
endif ()	

#
# TOPAS Options

option (TOPAS_COVERAGE "Enable coverage analysis" OFF)
option (TOPAS_MT "Enable multithreading support" ON)
option (TOPAS_WITH_STATIC_GEANT4 "Link against static Geant4 libraries" OFF)
option (TOPAS_WITH_PTL_GEANT4 "Link using PTL libraries from Geant4, no Visualization" OFF)

option (TOPAS_INSTALL_EXAMPLES "Install example TOPAS files" ON)

set (TOPAS_TYPE "expanded" CACHE STRING "Choose release type (expanded, educational or public")
if (NOT "${TOPAS_TYPE}" MATCHES "^(expanded|educational|public)$")
	message (FATAL_ERROR "Unrecognized TOPAS_TYPE=${TOPAS_TYPE}")
endif ()

if (TOPAS_WITH_STATIC_GEANT4)
	message (WARNING "Re-Linking of TOPAS with custom extensions from the install directory is not supported with static Geant4 linking yet!")
	message (WARNING "Qt deployment does not function the usual way with static builds. Build with shared first, deploy, and then re-build with static.")
endif ()
