cmake_minimum_required(VERSION 2.8)

project(mochapp CXX)

enable_testing()
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
if(UNIX)
target_link_libraries(${PROJECT_NAME} INTERFACE -pthread)
endif()
add_subdirectory(samples)

#add_subdirectory(mochawatcher)
