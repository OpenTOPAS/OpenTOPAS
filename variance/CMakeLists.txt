file(GLOB TOPAS_VARIANCE_SRC CONFIGURE_DEPENDS
    "*.hh"
    "*.cc"
)

add_library(variance ${TOPAS_VARIANCE_SRC})
