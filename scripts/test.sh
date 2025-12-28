#!/bin/bash

# libzen test script
set -e

echo "=== libzen Test Script ==="

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "Build directory not found. Building first..."
    bash scripts/build.sh
fi

cd build

# Run tests
echo "Running tests..."
ctest -V

echo "=== All tests completed! ==="