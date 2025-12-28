# Buffer module
set(BUFFER_SOURCES
    buffer.cpp
)

set(BUFFER_HEADERS
    buffer.h
    dynamic_buffer.h
    ring_buffer.h
)

add_library(zen_buffer OBJECT ${BUFFER_SOURCES})

# Add to parent scope
set(LIBZEN_OBJECTS ${LIBZEN_OBJECTS} $<TARGET_OBJECTS:zen_buffer> PARENT_SCOPE)
set(LIBZEN_HEADERS ${LIBZEN_HEADERS} ${BUFFER_HEADERS} PARENT_SCOPE)
