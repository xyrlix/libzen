# Video module
set(VIDEO_SOURCES
    video.cpp
)

set(VIDEO_HEADERS
    video.h
    video_codec.h
    video_process.h
)

add_library(zen_video OBJECT ${VIDEO_SOURCES})

# Add to parent scope
set(LIBZEN_OBJECTS ${LIBZEN_OBJECTS} $<TARGET_OBJECTS:zen_video> PARENT_SCOPE)
set(LIBZEN_HEADERS ${LIBZEN_HEADERS} ${VIDEO_HEADERS} PARENT_SCOPE)
