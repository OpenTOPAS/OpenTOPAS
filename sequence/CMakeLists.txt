option(TOPAS_USE_QT "Force-enable Qt UI build (otherwise auto-detected from Geant4)" OFF)
option(TOPAS_USE_QT6 "Force-enable Qt6 when Qt UI is enabled" OFF)

set(TOPAS_SEQUENCE_SRC
	TsActionInitialization.cc
	TsEventAction.cc
	TsSequenceManager.cc
	TsStackingAction.cc
	TsSteppingAction.cc
	TsTrackInformation.cc
	TsTrackingAction.cc
)

set(TOPAS_SEQUENCE_HEADERS
	TsActionInitialization.hh
	TsEventAction.hh
	TsSequenceManager.hh
	TsStackingAction.hh
	TsSteppingAction.hh
	TsTrackInformation.hh
	TsTrackingAction.hh
	TsQt.hh
)

set(TOPAS_USE_QT_DETECTED OFF)
set(TOPAS_USE_QT6_DETECTED OFF)

# Geant4 exports either CMake options (GEANT4_USE_QT/GEANT4_USE_QT_QT6)
# or compile definitions (G4UI_USE_QT/G4UI_USE_QT6). Look at all of them so
# Qt5 builds that only define G4UI_USE_QT are detected.
if (GEANT4_USE_QT AND GEANT4_USE_QT_QT6)
	set(TOPAS_USE_QT_DETECTED ON)
	set(TOPAS_USE_QT6_DETECTED ON)
elseif (GEANT4_USE_QT)
	set(TOPAS_USE_QT_DETECTED ON)
endif ()

set(_qt_def_sources ${Geant4_DEFINITIONS} ${CMAKE_CXX_FLAGS})
if (CMAKE_BUILD_TYPE)
	string(TOUPPER "${CMAKE_BUILD_TYPE}" _bt_upper)
	if (DEFINED CMAKE_CXX_FLAGS_${_bt_upper})
		list(APPEND _qt_def_sources ${CMAKE_CXX_FLAGS_${_bt_upper}})
	endif ()
endif ()

foreach(_entry IN LISTS _qt_def_sources)
	if (NOT TOPAS_USE_QT6_DETECTED AND "${_entry}" MATCHES "G4UI_USE_QT6")
		set(TOPAS_USE_QT_DETECTED ON)
		set(TOPAS_USE_QT6_DETECTED ON)
	elseif (NOT TOPAS_USE_QT_DETECTED AND "${_entry}" MATCHES "G4UI_USE_QT")
		set(TOPAS_USE_QT_DETECTED ON)
	endif ()
endforeach()

if (TOPAS_USE_QT6_DETECTED OR TOPAS_USE_QT6)
	set(TOPAS_USE_QT ON)
	set(TOPAS_USE_QT6 ON)
endif()

if (TOPAS_USE_QT_DETECTED AND NOT TOPAS_USE_QT)
	set(TOPAS_USE_QT ON)
endif()

if (TOPAS_USE_QT6)
	list(APPEND TOPAS_SEQUENCE_SRC TsQt6.cc)
	list(APPEND TOPAS_SEQUENCE_HEADERS TsQt6.hh)
elseif (TOPAS_USE_QT)
	list(APPEND TOPAS_SEQUENCE_SRC TsQt5.cc)
	list(APPEND TOPAS_SEQUENCE_HEADERS TsQt5.hh)
endif ()

add_library(sequence ${TOPAS_SEQUENCE_SRC} ${TOPAS_SEQUENCE_HEADERS})

if (TOPAS_USE_QT6)
	target_compile_definitions(sequence PRIVATE TOPAS_USE_QT6 G4UI_USE_QT G4UI_USE_QT6)
elseif (TOPAS_USE_QT)
	target_compile_definitions(sequence PRIVATE G4UI_USE_QT)
endif ()

target_link_libraries(sequence ${Geant4_LIBRARIES})
