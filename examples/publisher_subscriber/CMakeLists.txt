add_executable(publisher_node publisher_node.cpp)
target_link_libraries(publisher_node b0)

add_executable(subscriber_node subscriber_node.cpp)
target_link_libraries(subscriber_node b0)

