# IO module
set(IO_SOURCES
    io.cpp
)

set(IO_HEADERS
    io.h
    file_io.h
    async_io.h
)

add_library(zen_io OBJECT ${IO_SOURCES})

# Add to parent scope
set(LIBZEN_OBJECTS ${LIBZEN_OBJECTS} $<TARGET_OBJECTS:zen_io> PARENT_SCOPE)
set(LIBZEN_HEADERS ${LIBZEN_HEADERS} ${IO_HEADERS} PARENT_SCOPE)
