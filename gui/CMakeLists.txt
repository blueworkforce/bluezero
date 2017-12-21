find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)

add_executable(log_console_gui log_console_gui.cpp)
target_link_libraries(log_console_gui b0 Qt5::Core Qt5::Gui Qt5::Widgets)

add_executable(graph_console_gui graph_console_gui.cpp)
target_link_libraries(graph_console_gui b0 Qt5::Core Qt5::Gui Qt5::Widgets)

