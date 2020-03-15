cmake_minimum_required(VERSION 3.12)
project(ranges_to
    HOMEPAGE_URL https://github.com/cor3ntin/ranges_to
)

add_library(ranges_to INTERFACE)
target_compile_features(ranges_to INTERFACE cxx_std_20)
target_sources(ranges_to INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/cor3ntin/ranges_to/to.hpp)
target_include_directories(ranges_to INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/)


enable_testing()
add_subdirectory(test/Catch2)
add_executable(ranges_to_test EXCLUDE_FROM_ALL test/ranges_to.cpp)
target_link_libraries(ranges_to_test ranges_to Catch2)
add_test(NAME ranges_to COMMAND ranges_to_test)