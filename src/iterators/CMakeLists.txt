# Iterators module CMake configuration
add_library(zen_iterators INTERFACE)
target_include_directories(zen_iterators INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(zen_iterators INTERFACE zen_base)