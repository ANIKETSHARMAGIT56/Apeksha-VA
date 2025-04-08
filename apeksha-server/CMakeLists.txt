cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
add_executable(apeksha-server main.cpp)
add_subdirectory(Crow)
target_link_libraries(apeksha-server PUBLIC Crow)