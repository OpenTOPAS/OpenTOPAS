file(GLOB TOPAS_GRAPHICS_SRC CONFIGURE_DEPENDS
    "*.hh"
    "*.cc"
)

add_library(graphics ${TOPAS_GRAPHICS_SRC})
