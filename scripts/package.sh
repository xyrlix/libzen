#!/bin/bash

# libzen package script
set -e

echo "=== libzen Package Script ==="

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "Build directory not found. Building first..."
    bash scripts/build.sh
fi

cd build

# Create package directory
PACKAGE_DIR="libzen-$(date +%Y%m%d)-$(git rev-parse --short HEAD)"
mkdir -p "$PACKAGE_DIR"

# Copy header files
echo "Copying header files..."
mkdir -p "$PACKAGE_DIR/include"
cp -r ../include/ "$PACKAGE_DIR/"

# Copy library files
echo "Copying library files..."
mkdir -p "$PACKAGE_DIR/lib"
cp -v *.a "$PACKAGE_DIR/lib/" 2>/dev/null || true
cp -v *.so* "$PACKAGE_DIR/lib/" 2>/dev/null || true

# Copy CMake config files
echo "Copying CMake config files..."
mkdir -p "$PACKAGE_DIR/cmake"
cp -r ../cmake/ "$PACKAGE_DIR/"

# Copy documentation
echo "Copying documentation..."
mkdir -p "$PACKAGE_DIR/docs"
cp -r ../docs/ "$PACKAGE_DIR/"

# Copy examples
echo "Copying examples..."
mkdir -p "$PACKAGE_DIR/examples"
cp -r ../examples/ "$PACKAGE_DIR/"

# Copy LICENSE and README
echo "Copying LICENSE and README..."
cp ../LICENSE "$PACKAGE_DIR/"
cp ../README.md "$PACKAGE_DIR/"

# Copy build scripts
echo "Copying build scripts..."
mkdir -p "$PACKAGE_DIR/scripts"
cp ../scripts/build.sh "$PACKAGE_DIR/scripts/"
cp ../scripts/install.sh "$PACKAGE_DIR/scripts/"
cp ../scripts/test.sh "$PACKAGE_DIR/scripts/"

# Copy CMakeLists.txt
echo "Copying CMakeLists.txt..."
cp ../CMakeLists.txt "$PACKAGE_DIR/"

# Create tar.gz package
echo "Creating tar.gz package..."
tar -czf "$PACKAGE_DIR.tar.gz" "$PACKAGE_DIR"

# Clean up
echo "Cleaning up..."
rm -rf "$PACKAGE_DIR"

echo "=== Package created successfully! ==="
echo "Package: $PACKAGE_DIR.tar.gz"
echo "You can distribute this package to others."