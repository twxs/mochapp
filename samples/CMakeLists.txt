cmake_minimum_required(VERSION 3.0)

add_executable(sample main.cpp)
target_link_libraries(sample PRIVATE mochapp)