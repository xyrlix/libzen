#!/bin/bash

# libzen build script
set -e

echo "=== libzen Build Script ==="

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Configure with CMake
echo "[1/3] Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
echo "[2/3] Building..."
make -j$(nproc)

echo "[3/3] Build completed successfully!"
echo "Build artifacts are in the 'build' directory."
echo "Run 'bash scripts/test.sh' to run tests."
echo "Run 'sudo bash scripts/install.sh' to install."