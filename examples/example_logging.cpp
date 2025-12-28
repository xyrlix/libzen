#include <zen/logging.h>

int main() {
    // Initialize logger
    zen::logger& logger = zen::logger::get_instance();
    
    // Log messages with different levels
    logger.debug("This is a debug message");
    logger.info("This is an info message");
    logger.warn("This is a warning message");
    logger.error("This is an error message");
    logger.fatal("This is a fatal message");
    
    // Log with formatted strings
    int value = 42;
    logger.info("The answer is {}", value);
    
    // Log with multiple arguments
    logger.debug("Multiple values: {}, {}, {}", 1, 2.5, "hello");
    
    return 0;
}