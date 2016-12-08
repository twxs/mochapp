cmake_minimum_required(VERSION 3.0)

project(mochapp)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_subdirectory(samples)
