# Blockchain module
set(BLOCKCHAIN_SOURCES
    blockchain.cpp
)

set(BLOCKCHAIN_HEADERS
    blockchain.h
    block.h
    transaction.h
)

add_library(zen_blockchain OBJECT ${BLOCKCHAIN_SOURCES})

# Add to parent scope
set(LIBZEN_OBJECTS ${LIBZEN_OBJECTS} $<TARGET_OBJECTS:zen_blockchain> PARENT_SCOPE)
set(LIBZEN_HEADERS ${LIBZEN_HEADERS} ${BLOCKCHAIN_HEADERS} PARENT_SCOPE)
