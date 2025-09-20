add_library(matrix SHARED plugin_matrix.cpp)
add_executable(host host_main.cpp)
target_link_libraries(host plugin_manager)