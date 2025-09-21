add_library(plugin_stack_manager SHARED plugin_stack_manager.cpp)
add_executable(push push.cpp)
target_link_libraries(push plugin_manager)