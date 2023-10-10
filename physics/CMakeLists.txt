file(GLOB TOPAS_PHYSICS_SRC CONFIGURE_DEPENDS
    "*.hh"
    "*.icc"
    "*.cc"
)

add_library(physics ${TOPAS_PHYSICS_SRC})
