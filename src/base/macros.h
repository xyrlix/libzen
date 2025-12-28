#ifndef ZEN_BASE_MACROS_H
#define ZEN_BASE_MACROS_H

// Compilation macros
#ifdef NDEBUG
#define ZEN_DEBUG 0
#else
#define ZEN_DEBUG 1
#endif

// Platform macros
#ifdef __linux__
#define ZEN_LINUX 1
#else
#define ZEN_LINUX 0
#endif

// Utility macros
#define ZEN_UNUSED(x) (void)(x)
#define ZEN_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#endif // ZEN_BASE_MACROS_H