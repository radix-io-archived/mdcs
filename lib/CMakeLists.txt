set(MDCS_SRC
		${MDCS_SOURCE_DIR}/src/mdcs-service.c
		${MDCS_SOURCE_DIR}/src/mdcs-counters.c
		${MDCS_SOURCE_DIR}/src/hash_string.c)
message(STATUS "MDCS_SRC = ${MDCS_SRC}")

add_library(mdcs ${MDCS_SRC})
