add_executable(test_server test_server.c)
target_link_libraries(test_server mdcs ${MERCURY_LIBRARIES} ${BMI_LIBRARIES} ${ABT_SNOOZER_LIBRARIES} ${ABT_LIBRARIES} ${MARGO_LIBRARIES})

add_executable(test_client test_client.c)
target_link_libraries(test_client mdcs ${MERCURY_LIBRARIES} ${BMI_LIBRARIES} ${ABT_SNOOZER_LIBRARIES} ${ABT_LIBRARIES} ${MARGO_LIBRARIES})
