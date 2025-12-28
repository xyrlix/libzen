# Audio module
set(AUDIO_SOURCES
    audio.cpp
)

set(AUDIO_HEADERS
    audio.h
    audio_codec.h
    audio_process.h
)

add_library(zen_audio OBJECT ${AUDIO_SOURCES})

# Add to parent scope
set(LIBZEN_OBJECTS ${LIBZEN_OBJECTS} $<TARGET_OBJECTS:zen_audio> PARENT_SCOPE)
set(LIBZEN_HEADERS ${LIBZEN_HEADERS} ${AUDIO_HEADERS} PARENT_SCOPE)
