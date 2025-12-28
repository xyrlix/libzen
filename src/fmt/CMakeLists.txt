# Fmt module CMake configuration
add_library(zen_fmt STATIC fmt.cpp)
target_include_directories(zen_fmt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(zen_fmt PRIVATE zen_base zen_containers)