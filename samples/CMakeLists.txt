cmake_minimum_required(VERSION 2.8)

add_executable(sample main.cpp)
target_link_libraries(sample PRIVATE mochapp)
target_compile_features(sample PRIVATE cxx_range_for)
 add_test(test sample)
