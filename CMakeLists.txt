
cmake_minimum_required(VERSION 3.10)
project(SharedMemoryExample LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_TOOLCHAIN_FILE "C:\Users\like\Desktop\vcpkg\scripts\buildsystems\vcpkg.cmake")
find_package(Boost REQUIRED)

add_library(ipc_queue ipc_queue.cpp)
target_include_directories(ipc_queue PRIVATE ${Boost_INCLUDE_DIRS})
if(UNIX AND NOT APPLE)
    target_link_libraries(ipc_queue PRIVATE rt pthread)
elseif(APPLE)
    # macOS 不需要 rt
    target_link_libraries(ipc_queue PRIVATE pthread)
endif()

add_executable(writer writer.cpp)
target_link_libraries(writer ipc_queue)

add_executable(reader reader.cpp)
target_link_libraries(reader ipc_queue)
