set(${MDCS_SRC}
		${MCDS_SOURCE_DIR}/src/mdcs-service.c
		${MCDS_SOURCE_DIR}/src/mdcs-counters.c
		${MCDS_SOURCE_DIR}/src/hash_string.c)

add_library(mdcs ${MDCS_SRC})
