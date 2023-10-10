file(GLOB TOPAS_FILTERING_SRC CONFIGURE_DEPENDS
    "*.hh"
    "*.cc"
)

add_library(filtering ${TOPAS_FILTERING_SRC})
