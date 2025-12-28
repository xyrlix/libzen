# Storage module
set(STORAGE_SOURCES
    storage.cpp
)

set(STORAGE_HEADERS
    storage.h
    key_value_store.h
    file_storage.h
)

add_library(zen_storage OBJECT ${STORAGE_SOURCES})

# Add to parent scope
set(LIBZEN_OBJECTS ${LIBZEN_OBJECTS} $<TARGET_OBJECTS:zen_storage> PARENT_SCOPE)
set(LIBZEN_HEADERS ${LIBZEN_HEADERS} ${STORAGE_HEADERS} PARENT_SCOPE)
