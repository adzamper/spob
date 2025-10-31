@echo off
REM Build script for compiling sphere_overburden.cpp to WebAssembly
REM This script should be run from the Emscripten command prompt

echo Building Sphere Overburden WASM module...

REM Ensure output directory exists
if not exist "frontend\pkg" mkdir "frontend\pkg"

REM Compile with Emscripten
emcc sphere_overburden.cpp ^
  -o frontend/pkg/sphere_overburden_wasm.js ^
  -s WASM=1 ^
  -s EXPORT_ES6=1 ^
  -s MODULARIZE=1 ^
  -s EXPORT_NAME="createModule" ^
  -s ALLOW_MEMORY_GROWTH=1 ^
  -s MAXIMUM_MEMORY=4GB ^
  -O3 ^
  -s NO_EXIT_RUNTIME=1 ^
  -s ASSERTIONS=0 ^
  --bind

if %ERRORLEVEL% NEQ 0 (
    echo Build failed!
    exit /b %ERRORLEVEL%
)

echo Build successful!
echo Output files:
echo   - frontend/pkg/sphere_overburden_wasm.js
echo   - frontend/pkg/sphere_overburden_wasm.wasm
