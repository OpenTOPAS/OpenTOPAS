file(GLOB TOPAS_PARAMETER_SRC CONFIGURE_DEPENDS
    "*.hh"
    "*.cc"
)

add_library(parameter ${TOPAS_PARAMETER_SRC})