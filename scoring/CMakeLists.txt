file(GLOB TOPAS_SCORING_SRC CONFIGURE_DEPENDS
    "*.hh"
    "*.cc"
)

add_library(scoring ${TOPAS_SCORING_SRC})
