file(GLOB TOPAS_IO_SRC CONFIGURE_DEPENDS
    "*.hh"
    "*.cc"
)

add_library(io ${TOPAS_IO_SRC})
