# Base module CMake configuration
add_library(zen_base STATIC base.cpp)
target_include_directories(zen_base PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})