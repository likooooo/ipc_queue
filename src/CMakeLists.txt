find_package(Boost REQUIRED)

add_library(ipc_queue SHARED ipc_queue.cpp)
target_include_directories(ipc_queue PRIVATE ${Boost_INCLUDE_DIRS})
if(UNIX AND NOT APPLE)
    target_link_libraries(ipc_queue PRIVATE rt pthread)
elseif(APPLE)
    # macOS 不需要 rt
    target_link_libraries(ipc_queue PRIVATE pthread)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(plugin_manager)
add_subdirectory(tools)

# add_executable(writer writer.cpp)
# target_link_libraries(writer ipc_queue)

# add_executable(reader reader.cpp)
# target_link_libraries(reader ipc_queue)


# add_executable(sim_example_tool sim_example_tool.cpp)
# target_link_libraries(sim_example_tool ipc_queue)
# add_executable(sim_toykit sim_toykit.cpp)
# target_link_libraries(sim_toykit ipc_queue)