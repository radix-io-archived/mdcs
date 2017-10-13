add_executable(test_server test_server.c range-tracker.c)
target_link_libraries(test_server mdcs)

add_executable(test_client test_client.c)
target_link_libraries(test_client mdcs)
