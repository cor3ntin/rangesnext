cmake_minimum_required(VERSION 3.12)
project(rangesnext
    HOMEPAGE_URL https://github.com/cor3ntin/rangesnext
)

add_library(rangesnext INTERFACE)
target_compile_features(rangesnext INTERFACE cxx_std_20)
target_sources(rangesnext INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/cor3ntin/rangesnext/to.hpp)
target_include_directories(rangesnext INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/)


enable_testing()
add_subdirectory(test/Catch2)
add_executable(rangesnext_test EXCLUDE_FROM_ALL test/ranges_to.cpp)
target_link_libraries(rangesnext_test rangesnext Catch2)
add_test(NAME rangesnext COMMAND rangesnext_test)