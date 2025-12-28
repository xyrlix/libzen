# Tests CMake configuration

# Find Google Test
find_package(GTest REQUIRED)

# Test executable for base module
add_executable(test_base test_base.cpp)
target_link_libraries(test_base PRIVATE GTest::GTest GTest::Main zen_base)
add_test(NAME test_base COMMAND test_base)

# Test executable for memory module
add_executable(test_memory test_memory.cpp)
target_link_libraries(test_memory PRIVATE GTest::GTest GTest::Main zen_memory)
add_test(NAME test_memory COMMAND test_memory)

# Test executable for containers module
add_executable(test_containers
    test_containers/test_vector.cpp
    test_containers/test_unordered_map.cpp
)
target_link_libraries(test_containers PRIVATE GTest::GTest GTest::Main zen_containers)
add_test(NAME test_containers COMMAND test_containers)

# Test executable for algorithms module
add_executable(test_algorithms test_algorithms.cpp)
target_link_libraries(test_algorithms PRIVATE GTest::GTest GTest::Main zen_algorithms)
add_test(NAME test_algorithms COMMAND test_algorithms)

# Test executable for threading module
add_executable(test_threading test_threading.cpp)
target_link_libraries(test_threading PRIVATE GTest::GTest GTest::Main zen_threading)
add_test(NAME test_threading COMMAND test_threading)

# Test executable for logging module
add_executable(test_logging test_logging.cpp)
target_link_libraries(test_logging PRIVATE GTest::GTest GTest::Main zen_logging)
add_test(NAME test_logging COMMAND test_logging)

# Test executable for net module
add_executable(test_net test_net.cpp)
target_link_libraries(test_net PRIVATE GTest::GTest GTest::Main zen_net)
add_test(NAME test_net COMMAND test_net)