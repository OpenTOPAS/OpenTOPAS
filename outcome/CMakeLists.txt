file(GLOB TOPAS_OUTCOME_SRC CONFIGURE_DEPENDS
    "*.hh"
    "*.cc"
)

add_library(outcome ${TOPAS_OUTCOME_SRC})
