add_executable(pubsub pubsub.cpp)
target_link_libraries(pubsub b0)
add_test(pubsub pubsub 0)
add_test(pubsub_with_compression pubsub 1)

add_executable(pubsub_slowsub_conflate pubsub_slowsub.cpp)
target_link_libraries(pubsub_slowsub_conflate b0)
add_test(pubsub_slowsub_conflate_0 pubsub_slowsub_conflate 0)
add_test(pubsub_slowsub_conflate_1 pubsub_slowsub_conflate 1)

add_executable(pubsub_slowsub_conflate_zmq pubsub_slowsub_zmq.cpp)
target_link_libraries(pubsub_slowsub_conflate_zmq b0)
add_test(pubsub_slowsub_conflate_zmq_0 pubsub_slowsub_conflate_zmq 0)
add_test(pubsub_slowsub_conflate_zmq_1 pubsub_slowsub_conflate_zmq 1)

add_executable(pubsub2 pubsub2.cpp)
target_link_libraries(pubsub2 b0)
add_test(pubsub2 pubsub2)

add_executable(pubsub_unmanaged pubsub_unmanaged.cpp)
target_link_libraries(pubsub_unmanaged b0)
add_test(pubsub_unmanaged pubsub_unmanaged)

add_executable(pubsub_raw pubsub_raw.cpp)
target_link_libraries(pubsub_raw b0)
add_test(pubsub_raw pubsub_raw)

add_executable(pubsub_many pubsub_many.cpp)
target_link_libraries(pubsub_many b0)
add_test(pubsub_many pubsub_many)

add_executable(clisrv clisrv.cpp)
target_link_libraries(clisrv b0)
add_test(clisrv clisrv)

add_executable(clisrv2 clisrv2.cpp)
target_link_libraries(clisrv2 b0)
add_test(clisrv2 clisrv2)

add_executable(clisrv_raw clisrv_raw.cpp)
target_link_libraries(clisrv_raw b0)
add_test(clisrv_raw clisrv_raw)

add_executable(clisrv_badname clisrv_badname.cpp)
target_link_libraries(clisrv_badname b0)
add_test(clisrv_badname clisrv_badname)

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

add_executable(announce_timeout announce_timeout.cpp)
target_link_libraries(announce_timeout b0)
add_test(announce_timeout announce_timeout 1000 1)
add_test(announce_notimeout announce_timeout -1 0)

add_executable(two_nodes_one_thread two_nodes_one_thread.cpp)
target_link_libraries(two_nodes_one_thread b0)
add_test(two_nodes_one_thread two_nodes_one_thread)

add_executable(check_resolver_status check_resolver_status.cpp)
target_link_libraries(check_resolver_status b0)
add_test(check_resolver_status_when_not_running check_resolver_status 0)
add_test(check_resolver_status_when_running check_resolver_status 1)

add_executable(protocol protocol.cpp)
target_link_libraries(protocol b0)
add_test(protocol protocol)
