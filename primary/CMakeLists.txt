file(GLOB TOPAS_PRIMARY_SRC CONFIGURE_DEPENDS
    "*.hh"
    "*.cc"
)

add_library(primary ${TOPAS_PRIMARY_SRC})