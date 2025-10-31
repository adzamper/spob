# Windows Setup Guide - Sphere-Overburden EM Calculator

This guide provides detailed step-by-step instructions for setting up and running the Sphere-Overburden Electromagnetic Response Calculator on Windows.

## Table of Contents
1. [Prerequisites](#prerequisites)
2. [Installing Emscripten SDK](#installing-emscripten-sdk)
3. [Building the WASM Module](#building-the-wasm-module)
4. [Running the Web Application](#running-the-web-application)
5. [Troubleshooting](#troubleshooting)

---

## Prerequisites

### Required Software

1. **Git for Windows** (optional, but recommended)
   - Download from: https://git-scm.com/download/win
   - Install with default options

2. **Python 3.x** (for running local web server)
   - Download from: https://www.python.org/downloads/
   - During installation, **check the box "Add Python to PATH"**
   - Verify installation:
     ```cmd
     python --version
     ```

3. **Visual Studio Build Tools** (optional, for native C++ testing)
   - Download from: https://visualstudio.microsoft.com/downloads/
   - Select "Desktop development with C++" workload

---

## Installing Emscripten SDK

### Step 1: Download Emscripten

Open **Command Prompt** or **PowerShell** and run:

```cmd
cd C:\
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
```

If you don't have Git, download the ZIP file from https://github.com/emscripten-core/emsdk/archive/refs/heads/main.zip and extract it to `C:\emsdk`.

### Step 2: Install Emscripten

```cmd
cd C:\emsdk
emsdk install latest
```

This will download and install:
- Emscripten compiler toolchain
- Node.js (if not already installed)
- LLVM compiler infrastructure

**Note:** This download is approximately 1-2 GB and may take 10-30 minutes depending on your internet connection.

### Step 3: Activate Emscripten

```cmd
emsdk activate latest
```

### Step 4: Set Up Environment Variables

**For the current terminal session only:**
```cmd
emsdk_env.bat
```

**To make Emscripten available in all terminal sessions (optional):**

1. Right-click "This PC" → Properties → Advanced System Settings
2. Click "Environment Variables"
3. Under "System Variables", edit `Path`
4. Add: `C:\emsdk`
5. Add: `C:\emsdk\upstream\emscripten`
6. Click OK to save

### Step 5: Verify Installation

```cmd
emcc --version
```

You should see output like:
```
emcc (Emscripten gcc/clang-like replacement + linker emulating GNU ld) 3.1.XX
```

---

## Building the WASM Module

### Step 1: Navigate to Your Project Directory

```cmd
cd C:\Users\YourUsername\Documents\spob
```

Replace `C:\Users\YourUsername\Documents\spob` with the actual path to your project.

### Step 2: Activate Emscripten Environment

**IMPORTANT:** You must do this EVERY time you open a new command prompt!

```cmd
C:\emsdk\emsdk_env.bat
```

You should see:
```
Adding directories to PATH:
PATH += C:\emsdk
PATH += C:\emsdk\upstream\emscripten
...
```

### Step 3: Create Output Directory

```cmd
mkdir frontend\pkg
```

(If the directory already exists, this command will show an error but it's safe to ignore)

### Step 4: Run the Build Script

```cmd
build_wasm.bat
```

You should see:
```
Building Sphere Overburden WASM module...
Build successful!
Output files:
  - frontend/pkg/sphere_overburden_wasm.js
  - frontend/pkg/sphere_overburden_wasm.wasm
```

### Step 5: Verify Build Output

```cmd
dir frontend\pkg
```

You should see at least these files:
- `sphere_overburden_wasm.js` (approximately 10-15 KB)
- `sphere_overburden_wasm.wasm` (approximately 100-150 KB)

---

## Running the Web Application

### Method 1: Python HTTP Server (Recommended)

1. Open a **new** Command Prompt (don't need Emscripten for this)

2. Navigate to the frontend directory:
   ```cmd
   cd C:\Users\YourUsername\Documents\spob\frontend
   ```

3. Start the Python web server:
   ```cmd
   python -m http.server 8000
   ```

4. You should see:
   ```
   Serving HTTP on :: port 8000 (http://[::]:8000/) ...
   ```

5. Open your web browser and go to:
   ```
   http://localhost:8000
   ```

6. To stop the server, press `Ctrl+C` in the Command Prompt

### Method 2: Node.js http-server

1. Install http-server globally (one time only):
   ```cmd
   npm install -g http-server
   ```

2. Navigate to frontend directory:
   ```cmd
   cd C:\Users\YourUsername\Documents\spob\frontend
   ```

3. Start the server:
   ```cmd
   http-server -p 8000
   ```

4. Open browser to: http://localhost:8000

### Method 3: PHP Built-in Server

If you have PHP installed:

```cmd
cd C:\Users\YourUsername\Documents\spob\frontend
php -S localhost:8000
```

### Method 4: Visual Studio Code Live Server

1. Install VS Code from: https://code.visualstudio.com/

2. Install the "Live Server" extension:
   - Open VS Code
   - Go to Extensions (Ctrl+Shift+X)
   - Search for "Live Server" by Ritwick Dey
   - Click Install

3. Open your project:
   - File → Open Folder
   - Select the `spob` folder

4. Start Live Server:
   - Navigate to `frontend/index.html` in the file explorer
   - Right-click on `index.html`
   - Select "Open with Live Server"

5. Your browser will automatically open to the application

---

## Troubleshooting

### Problem: `emcc: command not found` or `'emcc' is not recognized`

**Solution:**
```cmd
C:\emsdk\emsdk_env.bat
```

You must run this every time you open a new terminal. Alternatively, add Emscripten to your system PATH permanently (see Step 4 under Installing Emscripten SDK).

---

### Problem: `Build failed!` when running build_wasm.bat

**Possible causes:**

1. **Emscripten not activated** - Run `C:\emsdk\emsdk_env.bat`

2. **Output directory doesn't exist** - Run:
   ```cmd
   mkdir frontend\pkg
   ```

3. **Wrong directory** - Make sure you're in the root `spob` folder where `sphere_overburden.cpp` is located

4. **Syntax errors in C++ code** - Check the error output for details

---

### Problem: Browser shows "Failed to load WASM module"

**Check 1:** Verify WASM files exist
```cmd
dir frontend\pkg\sphere_overburden_wasm.*
```

You should see:
- `sphere_overburden_wasm.js`
- `sphere_overburden_wasm.wasm`

**Check 2:** Make sure you're using a web server

❌ **WRONG:** Opening the file directly
```
file:///C:/Users/YourUsername/Documents/spob/frontend/index.html
```

✅ **CORRECT:** Using a web server
```
http://localhost:8000
```

**Check 3:** Check browser console for errors
- Press F12 in your browser
- Go to the "Console" tab
- Look for red error messages
- Common errors:
  - `CORS error` → You're not using a web server
  - `404 Not Found` → WASM files missing or in wrong location
  - `Module not found` → Path issue in code

---

### Problem: `wasmModule.calculate_em_response is not a function`

**This bug has been fixed.**

The issue was in `frontend/app.js` where the WASM module initialization wasn't properly capturing the initialized module instance.

**Solution:** Update to the latest version of `app.js` which fixes this initialization bug. Then rebuild:
```cmd
C:\emsdk\emsdk_env.bat
cd C:\Users\YourUsername\Documents\spob
build_wasm.bat
```

---

### Problem: `responseData.toJSON is not a function`

**This bug has been fixed.**

The issue was that the C++ function returns a JSON string directly, but the JavaScript code was trying to call `.toJSON()` on it as if it were an object.

**Solution:** Update to the latest version of `app.js`. The fix changes:
```javascript
// OLD (incorrect):
const response = JSON.parse(responseData.toJSON());

// NEW (correct):
const response = JSON.parse(responseJson);
```

After updating, refresh your browser with Ctrl+F5 to clear the cache.

---

### Problem: Calculation takes very long or browser freezes

**This is normal behavior for the first calculation.** The computation is intensive:
- 15 time windows
- 101 profile positions
- Complex mathematical operations

Expect 5-20 seconds for a complete calculation, depending on your computer.

**Tips:**
- Be patient and wait for the first calculation
- Check the browser's developer console (F12) for progress messages
- Make sure your parameter values are reasonable (avoid extreme values)

---

### Problem: Results seem incorrect or unexpected

**Check your parameter units:**
- All inputs must be in SI units:
  - Distance: **meters** (not km or feet)
  - Conductivity: **Siemens/meter** (not mS/m)
  - Time: **seconds**
  - Dipole moment: **Ampere·meter²**

**Check coordinate system:**
- X: along profile
- Y: perpendicular to profile
- **Z: positive DOWNWARD** (depth below surface)

**Check overburden parameters:**
- Overburden conductance = conductivity × thickness
- If thickness = 0, overburden effects are disabled

---

### Problem: Python not found

**Solution 1:** Install Python
- Download from https://www.python.org/downloads/
- During installation, check "Add Python to PATH"
- Restart Command Prompt

**Solution 2:** Use a different web server method (see Method 2, 3, or 4 above)

---

### Problem: Port 8000 already in use

**Solution:** Use a different port
```cmd
python -m http.server 8080
```

Then open: http://localhost:8080

---

## Quick Reference Card

### Every Time You Build WASM:

```cmd
REM 1. Open Command Prompt
REM 2. Activate Emscripten
C:\emsdk\emsdk_env.bat

REM 3. Navigate to project
cd C:\Users\YourUsername\Documents\spob

REM 4. Build
build_wasm.bat
```

### Every Time You Run the Application:

```cmd
REM 1. Open NEW Command Prompt (don't need Emscripten)
REM 2. Navigate to frontend
cd C:\Users\YourUsername\Documents\spob\frontend

REM 3. Start server
python -m http.server 8000

REM 4. Open browser to http://localhost:8000
```

---

## Advanced: Testing C++ Code Natively (Without WASM)

If you want to test the C++ code directly without compiling to WASM:

### Using Visual Studio Command Prompt:

```cmd
cl /EHsc /O2 /std:c++17 sphere_overburden.cpp
sphere_overburden.exe
```

### Using MinGW:

```cmd
g++ -std=c++11 -O3 sphere_overburden.cpp -o sphere_overburden.exe
sphere_overburden.exe
```

**Note:** The native version doesn't have the web interface - it just runs the calculation and exits.

---

## Next Steps

Once everything is working:

1. **Explore the interface** - Try different parameter combinations
2. **Read the scientific paper** - `desmarais2016.pdf` explains the physics
3. **Compare with MATLAB** - The `matcode/` folder has the original implementation
4. **Modify parameters** - Edit the default values in the HTML form
5. **Export results** - Use Plotly's built-in export features to save plots

---

## Getting Help

If you encounter issues:

1. **Check this troubleshooting section** - Most common issues are covered above
2. **Check browser console** - Press F12 and look for error messages
3. **Verify build output** - Make sure `build_wasm.bat` completed successfully
4. **Check file paths** - Make sure all files are in the correct locations
5. **Try rebuilding** - Delete `frontend/pkg/` and rebuild from scratch

---

## Summary of File Locations

```
C:\
└── emsdk\                          # Emscripten SDK installation
    ├── emsdk.bat
    ├── emsdk_env.bat              # Run this before building!
    └── ...

C:\Users\YourUsername\Documents\
└── spob\                           # Your project root
    ├── sphere_overburden.cpp      # C++ source code
    ├── build_wasm.bat             # Windows build script
    ├── build_wasm.sh              # Linux/Mac build script
    ├── README.md                  # General documentation
    ├── WINDOWS_SETUP.md          # This file
    ├── desmarais2016.pdf         # Scientific paper
    ├── matcode\                   # MATLAB reference code
    └── frontend\
        ├── index.html             # Web interface
        ├── app.js                 # JavaScript application
        ├── styles.css             # Styling
        └── pkg\                   # Generated by build
            ├── sphere_overburden_wasm.js    # WASM glue code
            └── sphere_overburden_wasm.wasm  # Compiled binary
```

---

**Last Updated:** 2025-10-31

For the latest version and updates, check the project repository.
