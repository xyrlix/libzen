# Memory module
set(MEMORY_SOURCES
    memory.cpp
)

set(MEMORY_HEADERS
    memory.h
    memory_pool.h
    smart_ptr.h
)

add_library(zen_memory OBJECT ${MEMORY_SOURCES})

# Add to parent scope
set(LIBZEN_OBJECTS ${LIBZEN_OBJECTS} $<TARGET_OBJECTS:zen_memory> PARENT_SCOPE)
set(LIBZEN_HEADERS ${LIBZEN_HEADERS} ${MEMORY_HEADERS} PARENT_SCOPE)
