#
# TOPAS libraries

install (FILES
    ${CMAKE_CURRENT_BINARY_DIR}/filtering/libfiltering.a
    ${CMAKE_CURRENT_BINARY_DIR}/chemistry/libchemistry.a
    ${CMAKE_CURRENT_BINARY_DIR}/geometry/libgeometry.a
    ${CMAKE_CURRENT_BINARY_DIR}/graphics/libgraphics.a
    ${CMAKE_CURRENT_BINARY_DIR}/io/libio.a
    ${CMAKE_CURRENT_BINARY_DIR}/main/libmain.a
    ${CMAKE_CURRENT_BINARY_DIR}/material/libmaterial.a
    ${CMAKE_CURRENT_BINARY_DIR}/outcome/liboutcome.a
    ${CMAKE_CURRENT_BINARY_DIR}/parameter/libparameter.a
    ${CMAKE_CURRENT_BINARY_DIR}/physics/libphysics.a
    ${CMAKE_CURRENT_BINARY_DIR}/primary/libprimary.a
    ${CMAKE_CURRENT_BINARY_DIR}/scoring/libscoring.a
    ${CMAKE_CURRENT_BINARY_DIR}/sequence/libsequence.a
    ${CMAKE_CURRENT_BINARY_DIR}/variance/libvariance.a
    DESTINATION lib)

# TOPAS headers
file (GLOB includesForEndUser
    "${CMAKE_CURRENT_BINARY_DIR}/TsTopasConfig.hh"
    "main/TsMain.hh"
    "parameter/TsParameterManager.hh"
    "geometry/TsGeometryManager.hh"
    "geometry/TsVGeometryComponent.hh"
    "geometry/TsVMagneticField.hh"
    "geometry/TsVElectroMagneticField.hh"
    "geometry/TsVImagingToMaterial.hh"
    "filtering/TsVFilter.hh"
    "primary/TsPrimaryParticle.hh"
    "primary/TsSource.hh"
    "primary/TsVGenerator.hh"
    "io/TsVFile.hh"
    "io/TsVNtuple.hh"
    "scoring/TsVBinnedScorer.hh"
    "scoring/TsVNtupleScorer.hh"
    "scoring/TsVScorer.hh"
    "outcome/TsVOutcomeModel.hh"
    "physics/TsModularPhysicsList.hh"
    "physics/TsModularPhysicsList.icc"
    "sequence/TsTrackInformation.hh"
    "variance/TsSplitProcessG4DNA.hh")

install (FILES ${includesForEndUser} DESTINATION include)

install (FILES
    forenduser/CMakeLists.txt
    DESTINATION .)
