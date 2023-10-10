#
# MUST BE INCLUDED -NOT BEFORE- TOPAS_Options
#

#
# GEANT4

if (NOT TOPAS_WITH_STATIC_GEANT4)
	find_package (Geant4 REQUIRED ui_all vis_all)
else ()
	find_package (Geant4 REQUIRED ui_all vis_all static)
endif ()

message (STATUS "Found Geant4 v${Geant4_VERSION_MAJOR}.${Geant4_VERSION_MINOR} (patch ${Geant4_VERSION_PATCH})")

include(${Geant4_USE_FILE})

# Add PTL include directories for version >= 10.7
# Update, Don't include PTL but Geant4 11 includes which are missing from new Geant4 CMake files
if (((Geant4_VERSION_MAJOR VERSION_GREATER_EQUAL 10) 
	AND (Geant4_VERSION_MINOR VERSION_GREATER_EQUAL 7))
	OR (Geant4_VERSION_MAJOR VERSION_GREATER_EQUAL 11))
	if (TOPAS_WITH_PTL_GEANT4)
		include_directories (AFTER SYSTEM ${PTL_INCLUDE_DIR})
	else()
		add_definitions(${Geant4_DEFINITIONS})
		include_directories(AFTER SYSTEM ${Geant4_INCLUDE_DIRS})
	endif()
endif()

# Override MT if Geant4 version less than 10
if (Geant4_VERSION_MAJOR LESS 10)
	set (TOPAS_MT OFF)
	message (WARNING "Disabling TOPAS_MT (Geant4 version < 10)")
endif ()

#
# GDCM

find_package (GDCM REQUIRED)

message(STATUS "Found GDCM v${GDCM_VERSION_MAJOR}.${GDCM_VERSION_MINOR}")

include (${GDCM_USE_FILE})
