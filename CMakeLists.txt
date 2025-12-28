cmake_minimum_required(VERSION 3.16)
project(libzen VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include cmake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Include ZenUtils.cmake
include(cmake/ZenUtils.cmake)

# Add include directories
target_include_directories(zen_base PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add subdirectories
add_subdirectory(src/base)
add_subdirectory(src/memory)
add_subdirectory(src/containers)
add_subdirectory(src/iterators)
add_subdirectory(src/algorithms)
add_subdirectory(src/utility)
add_subdirectory(src/fmt)
add_subdirectory(src/threading)
add_subdirectory(src/io)
add_subdirectory(src/logging)
add_subdirectory(src/config)
add_subdirectory(src/filesystem)
add_subdirectory(src/net)
add_subdirectory(src/timer)
add_subdirectory(src/serialize)
add_subdirectory(src/math)
add_subdirectory(src/type)
add_subdirectory(src/error)
add_subdirectory(src/crypto)
add_subdirectory(src/process)
add_subdirectory(src/event)
add_subdirectory(src/proto)
add_subdirectory(src/system)
add_subdirectory(src/buffer)
add_subdirectory(src/database)
add_subdirectory(src/storage)
add_subdirectory(src/blockchain)
add_subdirectory(src/pool)
add_subdirectory(src/image)
add_subdirectory(src/audio)
add_subdirectory(src/video)

# Add tests if enabled
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Add examples if enabled
option(BUILD_EXAMPLES "Build examples" ON)
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Create the main library
add_library(zen INTERFACE)
target_link_libraries(zen INTERFACE
    zen_base
    zen_memory
    zen_containers
    zen_iterators
    zen_algorithms
    zen_utility
    zen_fmt
    zen_threading
    zen_io
    zen_logging
    zen_config
    zen_filesystem
    zen_net
    zen_timer
    zen_serialize
    zen_math
    zen_type
    zen_error
    zen_crypto
    zen_process
    zen_event
    zen_proto
    zen_system
    zen_buffer
    zen_database
    zen_storage
    zen_blockchain
    zen_pool
    zen_image
    zen_audio
    zen_video
)

# Install configuration
include(GNUInstallDirs)

# Install headers
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Install cmake config files
install(FILES cmake/ZenConfig.cmake cmake/ZenUtils.cmake DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/zen)

# Install library targets
install(TARGETS zen_base zen_memory zen_containers zen_iterators zen_algorithms zen_utility zen_fmt
                zen_threading zen_io zen_logging zen_config zen_filesystem zen_net zen_timer
                zen_serialize zen_math zen_type zen_error zen_crypto zen_process zen_event zen_proto
                zen_system zen_buffer zen_database zen_storage zen_blockchain
                zen_pool zen_image zen_audio zen_video
        EXPORT ZenTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Export targets
install(EXPORT ZenTargets FILE ZenTargets.cmake NAMESPACE zen:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/zen)
