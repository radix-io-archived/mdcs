set(MDCS_SRC
		${MDCS_SOURCE_DIR}/src/mdcs-service.c
		${MDCS_SOURCE_DIR}/src/mdcs-client.c
		${MDCS_SOURCE_DIR}/src/mdcs-counters.c
		${MDCS_SOURCE_DIR}/src/mdcs-rpc.c
		${MDCS_SOURCE_DIR}/src/mdcs-hash-string.c)

add_library(mdcs ${MDCS_SRC})
