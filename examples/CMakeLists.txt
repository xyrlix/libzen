# Examples CMake configuration

# Example for vector usage
add_executable(example_vector example_vector.cpp)
target_link_libraries(example_vector PRIVATE zen)

# Example for thread pool usage
add_executable(example_thread_pool example_thread_pool.cpp)
target_link_libraries(example_thread_pool PRIVATE zen)

# Example for logging usage
add_executable(example_logging example_logging.cpp)
target_link_libraries(example_logging PRIVATE zen)

# Example for TCP server usage
add_executable(example_tcp_server example_tcp_server.cpp)
target_link_libraries(example_tcp_server PRIVATE zen)

# Example for config usage
add_executable(example_config example_config.cpp)
target_link_libraries(example_config PRIVATE zen)