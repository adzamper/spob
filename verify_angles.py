#!/usr/bin/env python3
import math

def normalize(v):
    """Normalize a vector"""
    mag = math.sqrt(v[0]**2 + v[1]**2 + v[2]**2)
    return [v[0]/mag, v[1]/mag, v[2]/mag] if mag > 0 else [0, 0, 0]

def dot(a, b):
    """Dot product"""
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2]

def scale(v, s):
    """Scale vector"""
    return [v[0]*s, v[1]*s, v[2]*s]

def matlab_formula(dip, strike):
    """MATLAB formula for normal vector"""
    norm = [
        math.cos(math.radians(90 - dip)) * math.cos(math.radians(strike - 90)),
        math.sin(math.radians(strike - 90)) * math.cos(math.radians(90 - dip)),
        math.sin(math.radians(90 - dip))
    ]
    # Normalize
    norm = normalize(norm)
    return norm

def cpp_formula(dip, strike):
    """C++ formula for normal vector"""
    PI = math.pi
    strike_rad = (strike - 90.0) * PI / 180.0
    dip_rad = (90.0 - dip) * PI / 180.0

    norm = [
        math.cos(dip_rad) * math.cos(strike_rad),
        math.sin(strike_rad) * math.cos(dip_rad),
        math.sin(dip_rad)
    ]
    # Normalize
    norm = normalize(norm)
    return norm

# Test cases
test_cases = [
    (90, 90),  # Working case
    (93, 90),  # Failing case
    (85, 90),  # Another test
]

for dip, strike in test_cases:
    print(f"\n=== dip={dip}, strike={strike} ===")

    matlab_norm = matlab_formula(dip, strike)
    cpp_norm = cpp_formula(dip, strike)

    print(f"MATLAB norm: {matlab_norm}")
    print(f"C++ norm:    {cpp_norm}")

    diff = [matlab_norm[i] - cpp_norm[i] for i in range(3)]
    print(f"Difference:  {diff}")
    print(f"Max diff:    {max(abs(d) for d in diff)}")

    # Test with a sample moment
    msp = [100.0, 0.0, 50.0]
    print(f"\nOriginal moment: {msp}")

    # MATLAB projection
    matlab_proj_scale = dot(msp, matlab_norm)
    matlab_proj = scale(matlab_norm, matlab_proj_scale)
    print(f"MATLAB projected: {matlab_proj}")

    # C++ projection
    cpp_proj_scale = dot(msp, cpp_norm)
    cpp_proj = scale(cpp_norm, cpp_proj_scale)
    print(f"C++ projected:    {cpp_proj}")

    proj_diff = [matlab_proj[i] - cpp_proj[i] for i in range(3)]
    print(f"Projection diff:  {proj_diff}")
