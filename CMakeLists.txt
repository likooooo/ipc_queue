
cmake_minimum_required(VERSION 3.10)
project(SharedMemoryExample LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# include_directories(plugin)
add_subdirectory(src)
# add_subdirectory(plugin)