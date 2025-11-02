// Trace to verify the dip > 90° fix is working
#include <cmath>
#include <iostream>
#include <iomanip>

const double PI = 3.14159265358979323846;

int main() {
    std::cout << std::fixed << std::setprecision(6);

    double dip = 92.0;
    double strike = 90.0;

    std::cout << "Testing dip > 90° remapping:\n";
    std::cout << "============================\n\n";
    std::cout << "Original: dip=" << dip << "°, strike=" << strike << "°\n\n";

    // Apply remapping
    double effective_dip = dip;
    double effective_strike = strike;
    if (dip > 90.0) {
        effective_dip = 180.0 - dip;
        effective_strike = strike + 180.0;
        std::cout << "REMAPPED: effective_dip=" << effective_dip << "°, effective_strike=" << effective_strike << "°\n\n";
    }

    // Calculate angles
    double strike_rad = (effective_strike - 90.0) * PI / 180.0;
    double dip_rad = (90.0 - effective_dip) * PI / 180.0;

    std::cout << "Angles:\n";
    std::cout << "  strike_rad = (" << effective_strike << " - 90) * π/180 = " << strike_rad << " rad\n";
    std::cout << "  dip_rad = (90 - " << effective_dip << ") * π/180 = " << dip_rad << " rad\n\n";

    // Calculate normal
    double norm_x = std::cos(dip_rad) * std::cos(strike_rad);
    double norm_y = std::sin(strike_rad) * std::cos(dip_rad);
    double norm_z = std::sin(dip_rad);

    std::cout << "Normal vector:\n";
    std::cout << "  norm.x = cos(" << dip_rad << ") * cos(" << strike_rad << ") = " << norm_x << "\n";
    std::cout << "  norm.y = sin(" << strike_rad << ") * cos(" << dip_rad << ") = " << norm_y << "\n";
    std::cout << "  norm.z = sin(" << dip_rad << ") = " << norm_z << "\n\n";

    // Test projection with typical moment
    double msp_x = -3200.0;
    double msp_z = -150000.0;

    double dot_product = msp_x * norm_x + msp_z * norm_z;

    std::cout << "Projection test with msp=(" << msp_x << ", 0, " << msp_z << "):\n";
    std::cout << "  msp · norm = " << msp_x << " * " << norm_x << " + " << msp_z << " * " << norm_z << "\n";
    std::cout << "             = " << (msp_x * norm_x) << " + " << (msp_z * norm_z) << "\n";
    std::cout << "             = " << dot_product << "\n\n";

    std::cout << "Result: " << (dot_product < 0 ? "NEGATIVE ✓" : "POSITIVE ✗") << "\n\n";

    // Compare to dip=90
    std::cout << "Comparison to dip=90°:\n";
    std::cout << "  dip=90: norm=(1, 0, 0), projection = -3200\n";
    std::cout << "  dip=92 (fixed): norm=(" << norm_x << ", " << norm_y << ", " << norm_z << "), projection = " << dot_product << "\n";
    std::cout << "  Ratio: " << (dot_product / -3200.0) << "\n";

    return 0;
}
