add_executable(multi_publisher multi_publisher.cpp)
target_link_libraries(multi_publisher b0)

add_executable(multi_subscriber multi_subscriber.cpp)
target_link_libraries(multi_subscriber b0)

