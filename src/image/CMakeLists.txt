# Image module
set(IMAGE_SOURCES
    image.cpp
)

set(IMAGE_HEADERS
    image.h
    image_loader.h
    image_process.h
)

add_library(zen_image OBJECT ${IMAGE_SOURCES})

# Add to parent scope
set(LIBZEN_OBJECTS ${LIBZEN_OBJECTS} $<TARGET_OBJECTS:zen_image> PARENT_SCOPE)
set(LIBZEN_HEADERS ${LIBZEN_HEADERS} ${IMAGE_HEADERS} PARENT_SCOPE)
