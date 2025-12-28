# ZenConfig.cmake - Configuration for external projects

# Include the targets file
include(${CMAKE_CURRENT_LIST_DIR}/ZenTargets.cmake)

# Set include directories for users
set(ZEN_INCLUDE_DIRS ${CMAKE_INSTALL_PREFIX}/include)

# Export the main target
set(ZEN_LIBRARIES zen)

# Check if the package was found
set(ZEN_FOUND TRUE)

# Macro to easily link against Zen
macro(target_link_zen target)
    target_link_libraries(${target} PRIVATE zen::zen)
    target_include_directories(${target} PRIVATE ${ZEN_INCLUDE_DIRS})
endmacro()