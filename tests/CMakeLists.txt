protobuf_generate_cpp(TEST_MSGS TEST_MSGS_H test_msgs.proto)

add_executable(pubsub pubsub.cpp ${TEST_MSGS})
target_link_libraries(pubsub b0)
add_test(pubsub pubsub)

add_executable(pubsub2 pubsub2.cpp ${TEST_MSGS})
target_link_libraries(pubsub2 b0)
add_test(pubsub2 pubsub2)

add_executable(pubsub_raw pubsub_raw.cpp ${TEST_MSGS})
target_link_libraries(pubsub_raw b0)
add_test(pubsub_raw pubsub_raw)

add_executable(clisrv clisrv.cpp ${TEST_MSGS})
target_link_libraries(clisrv b0)
add_test(clisrv clisrv)

add_executable(clisrv2 clisrv2.cpp ${TEST_MSGS})
target_link_libraries(clisrv2 b0)
add_test(clisrv2 clisrv2)

add_executable(clisrv_raw clisrv_raw.cpp ${TEST_MSGS})
target_link_libraries(clisrv_raw b0)
add_test(clisrv_raw clisrv_raw)

add_executable(time_sync time_sync.cpp)
target_link_libraries(time_sync b0)
add_test(time_sync_offset_recover time_sync offset_recover)
add_test(time_sync_clock_tracking_1 time_sync clock_tracking 0.5 0.6 1)
add_test(time_sync_clock_tracking_2 time_sync clock_tracking 0.5 1.5 1)
add_test(time_sync_clock_tracking_3 time_sync clock_tracking 0.5 1.66 0)

add_executable(compress compress.cpp)
target_link_libraries(compress b0)
add_test(compress-zlib compress zlib)
add_test(compress-lz4 compress lz4)

