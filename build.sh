#!/bin/bash
# Crest Build Script for Linux/macOS

set -e

echo "🌊 Building Crest..."

# Clean previous builds
echo "Cleaning previous builds..."
rm -rf build .xmake

# Configure
echo "Configuring build..."
xmake config -m release

# Build library
echo "Building library..."
xmake build crest

# Build examples
echo "Building examples..."
xmake build crest_example_cpp
xmake build crest_example_c

# Build tests
echo "Building tests..."
xmake build crest_tests

echo "✅ Build complete!"
echo ""
echo "Run examples:"
echo "  xmake run crest_example_cpp"
echo "  xmake run crest_example_c"
echo ""
echo "Run tests:"
echo "  xmake run crest_tests"
