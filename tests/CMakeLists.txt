protobuf_generate_cpp(TEST_MSGS TEST_MSGS_H test_msgs.proto)

add_executable(pubsub pubsub.cpp ${TEST_MSGS})
target_link_libraries(pubsub b0)
add_test(pubsub pubsub)

add_executable(pubsub2 pubsub2.cpp ${TEST_MSGS})
target_link_libraries(pubsub2 b0)
add_test(pubsub2 pubsub2)

add_executable(clisrv clisrv.cpp ${TEST_MSGS})
target_link_libraries(clisrv b0)
add_test(clisrv clisrv)

add_executable(clisrv2 clisrv2.cpp ${TEST_MSGS})
target_link_libraries(clisrv2 b0)
add_test(clisrv2 clisrv2)

