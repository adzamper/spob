#!/bin/bash
# Build script for compiling sphere_overburden.cpp to WebAssembly
# Requires Emscripten SDK to be installed and activated

echo "Building Sphere Overburden WASM module..."

# Ensure frontend/pkg directory exists
mkdir -p frontend/pkg

# Compile with Emscripten
# NOTE: Removed -s ASSERTIONS=0 to enable console output
# Added -s ASSERTIONS=1 for better debugging
emcc sphere_overburden.cpp \
  -o frontend/pkg/sphere_overburden_wasm.js \
  -s WASM=1 \
  -s EXPORT_ES6=1 \
  -s MODULARIZE=1 \
  -s EXPORT_NAME="createModule" \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s MAXIMUM_MEMORY=4GB \
  -O3 \
  -s NO_EXIT_RUNTIME=1 \
  -s ASSERTIONS=1 \
  --bind

if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "Output files:"
    echo "  - frontend/pkg/sphere_overburden_wasm.js"
    echo "  - frontend/pkg/sphere_overburden_wasm.wasm"
else
    echo "Build failed!"
    exit 1
fi
