file(GLOB TOPAS_SEQUENCE_SRC CONFIGURE_DEPENDS
    "*.hh"
    "*.cc"
)

add_library(sequence ${TOPAS_SEQUENCE_SRC})

target_link_libraries(sequence ${Geant4_LIBRARIES})
