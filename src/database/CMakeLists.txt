# Database module
set(DATABASE_SOURCES
    database.cpp
)

set(DATABASE_HEADERS
    database.h
    connection.h
    result.h
)

add_library(zen_database OBJECT ${DATABASE_SOURCES})

# Add to parent scope
set(LIBZEN_OBJECTS ${LIBZEN_OBJECTS} $<TARGET_OBJECTS:zen_database> PARENT_SCOPE)
set(LIBZEN_HEADERS ${LIBZEN_HEADERS} ${DATABASE_HEADERS} PARENT_SCOPE)
