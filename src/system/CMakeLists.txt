# System module
set(SYSTEM_SOURCES
    system.cpp
)

set(SYSTEM_HEADERS
    system.h
    filesystem.h
    system_info.h
    system_utils.h
)

add_library(zen_system OBJECT ${SYSTEM_SOURCES})

# Add to parent scope
set(LIBZEN_OBJECTS ${LIBZEN_OBJECTS} $<TARGET_OBJECTS:zen_system> PARENT_SCOPE)
set(LIBZEN_HEADERS ${LIBZEN_HEADERS} ${SYSTEM_HEADERS} PARENT_SCOPE)
