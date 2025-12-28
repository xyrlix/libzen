#!/bin/bash

# libzen install script
set -e

echo "=== libzen Install Script ==="

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "Error: Build directory not found. Please run 'bash scripts/build.sh' first."
    exit 1
fi

cd build

# Install
echo "[1/2] Installing libzen..."
sudo make install

echo "[2/2] Installation completed successfully!"
echo "libzen has been installed to /usr/local/ by default."
echo "You can now use 'find_package(zen)' in your CMake projects."
echo "Run 'bash scripts/test.sh' to verify installation."