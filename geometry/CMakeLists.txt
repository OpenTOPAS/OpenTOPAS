file(GLOB TOPAS_GEOMETRY_SRC CONFIGURE_DEPENDS
    "*.hh"
    "*.cc"
)

add_library(geometry ${TOPAS_GEOMETRY_SRC})
