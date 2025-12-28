# Containers module CMake configuration
add_library(zen_containers STATIC
    sequential/vector.cpp
    sequential/list.cpp
    sequential/string.cpp
    associative/map.cpp
    associative/unordered_map.cpp
)
target_include_directories(zen_containers PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(zen_containers PRIVATE zen_base zen_memory)