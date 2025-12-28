# Utility module CMake configuration
add_library(zen_utility INTERFACE)
target_include_directories(zen_utility INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(zen_utility INTERFACE zen_base)