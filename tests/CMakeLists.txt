protobuf_generate_cpp(TEST_MSGS TEST_MSGS_H test_msgs.proto)

add_executable(pubsub pubsub.cpp ../src/resolver/resolver.cpp ${TEST_MSGS})
target_link_libraries(pubsub b0)

add_test(pubsub pubsub)

