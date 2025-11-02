# WASM Module Rebuild Required

## Issue Summary
The WASM module is producing incorrect values for dip=45 calculations because it's **out of date** with the latest C++ code fixes.

## Root Cause
Comparing debug output from WASM vs MATLAB revealed that the induced moment calculations differ significantly:

**MATLAB (correct):**
```
convo_z = +0.0276  (positive at rtx=144)
mspdotnorm = +0.0341
msp (after projection) = [+0.0241, 0, +0.0241]
```

**WASM (incorrect - from outdated module):**
```
convo_z = -0.00435  (negative at rtx=704)
mspdotnorm = -0.00170
msp (after projection) = [-0.00120, 0, -0.00120]
```

## File Timestamps
- ✅ `sphere_overburden.cpp` - **Nov 2, 2024** (contains fixes)
- ✅ `matcode/H_ob_z.m` - **Nov 1, 2024** (contains fixes from commit df80c63)
- ❌ `frontend/pkg/sphere_overburden_wasm_bg.wasm` - **Oct 31, 2024** (OUTDATED!)

## The Fix Applied (Already in C++ Code)
Commit df80c63 fixed z-component sign errors in the overburden field calculations:

### Before Fix:
```cpp
// WRONG: negative signs
double term1 = -m_z / r_3_2;
double term2 = ... (m_x * dx - m_z * dz + m_y * dy) ...
```

### After Fix:
```cpp
// CORRECT: positive signs
double term1 = m_z / r_3_2;
double term2 = ... (m_x * dx + m_z * dz + m_y * dy) ...
```

These fixes are present in:
- ✅ `sphere_overburden.cpp` lines 199, 202, 213, 216
- ✅ `matcode/H_ob_z.m`
- ✅ `matcode/dH_obdt_z.m`

## Solution: Rebuild WASM Module

Run on your local machine with Emscripten installed:

```bash
./build_wasm.sh
```

Or manually:
```bash
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
```

## Expected Results After Rebuild
Once rebuilt, WASM output should match MATLAB for dip=45 at rtx=144:

```
convo_x = 2.0700371320e-02
convo_z = 2.7578152768e-02  (positive!)
mspdotnorm = 3.4138071768e-02
msp (after) = [2.4139262044e-02, 0, 2.4139262044e-02]
H_tot_x = -5.9762297264e-11
H_tot_z = 1.9399159709e-10
```

## Additional Note: sigma_ob Value
The C++ default for `sigma_ob` is set to `1.0/30.0` (line 622), but MATLAB uses `1.0/300.0 = 0.00333333`. This may need adjustment depending on the intended default value, though it can be overridden via JSON parameters.

## Verification Steps
After rebuilding:
1. Load the updated WASM module in the web app
2. Run with dip=45, strike=90
3. Compare console debug output with MATLAB values above
4. Values should now match (within numerical precision)
