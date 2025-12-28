# ZenUtils.cmake - Utility functions and common settings for Zen

# Set common compile options
function(zen_set_compile_options target)
    # Set C++ standard
    target_compile_features(${target} PUBLIC cxx_std_17)
    
    # Warning levels
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /WX)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Werror -pedantic)
    endif()
    
    # Debug-specific options
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_definitions(${target} PRIVATE ZEN_DEBUG=1)
        if(MSVC)
            target_compile_options(${target} PRIVATE /Od /Zi)
        else()
            target_compile_options(${target} PRIVATE -O0 -g)
        endif()
    endif()
    
    # Release-specific options
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_definitions(${target} PRIVATE ZEN_DEBUG=0)
        if(MSVC)
            target_compile_options(${target} PRIVATE /O2 /DNDEBUG)
        else()
            target_compile_options(${target} PRIVATE -O3 -DNDEBUG)
        endif()
    endif()
endfunction()

# Check for required system libraries
function(zen_check_system_libraries)
    # Check for pthread
    find_package(Threads REQUIRED)
    
    # Check for system headers
    include(CheckIncludeFile)
    check_include_file(sys/socket.h ZEN_HAVE_SYS_SOCKET_H)
    check_include_file(netinet/in.h ZEN_HAVE_NETINET_IN_H)
    check_include_file(arpa/inet.h ZEN_HAVE_ARPA_INET_H)
    check_include_file(fcntl.h ZEN_HAVE_FCNTL_H)
    check_include_file(unistd.h ZEN_HAVE_UNISTD_H)
    check_include_file(sys/epoll.h ZEN_HAVE_SYS_EPOLL_H)
    
    # Check for functions
    include(CheckFunctionExists)
    check_function_exists(epoll_create1 ZEN_HAVE_EPOLL_CREATE1)
    check_function_exists(mmap ZEN_HAVE_MMAP)
    check_function_exists(munmap ZEN_HAVE_MUNMAP)
endfunction()

# Add a source group for better IDE organization
function(zen_add_source_group target sources)
    foreach(source_file ${sources})
        get_filename_component(source_dir ${source_file} DIRECTORY)
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" source_dir_rel ${source_dir})
        string(REPLACE "/" "\\" source_group ${source_dir_rel})
        source_group(${source_group} FILES ${source_file})
    endforeach()
endfunction()

# Generate a configuration header
function(zen_generate_config_header output_file)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/zen_config.h.in
        ${output_file}
        @ONLY
    )
endfunction()

# Check for C++ features
include(CheckCXXCompilerFlag)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Debug' as none was specified.")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

# Enable position-independent code for shared libraries
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Enable colors in Ninja output
if(CMAKE_GENERATOR STREQUAL "Ninja")
    if(MSVC)
        # Ninja with MSVC
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /nologo")
    else()
        # Ninja with GCC/Clang
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")
    endif()
endif()