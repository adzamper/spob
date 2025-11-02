// Test dip angle variation matching MATLAB parameters
#include <cmath>
#include <iostream>
#include <iomanip>
#include <functional>
#include <vector>
#include <limits>
#include <algorithm>

const double PI = 3.14159265358979323846;
const double MU_0 = 1.2566370614359172e-6;

struct Vec3 {
    double x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}
    Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
    Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
    Vec3 operator*(double s) const { return Vec3(x * s, y * s, z * s); }
    Vec3 operator/(double s) const { return Vec3(x / s, y / s, z / s); }
    double dot(const Vec3& v) const { return x * v.x + y * v.y + z * v.z; }
    double norm2() const { return x * x + y * y + z * z; }
    double norm() const { return std::sqrt(norm2()); }
    Vec3 normalized() const {
        double n = norm();
        return (n > 1e-20) ? (*this / n) : Vec3();
    }
};

Vec3 static_dipole_field(const Vec3& m, const Vec3& r) {
    double r2 = r.norm2();
    if (r2 < 1e-20) return Vec3(0, 0, 0);
    double one_over_4pi = 1.0 / (4.0 * PI);
    double r_mag = std::sqrt(r2);
    double a = one_over_4pi / (r_mag * r2);
    double b = r.dot(m) * 3.0 / r2;
    return (r * b - m) * a;
}

// Simplified calculation for sphere field only (no overburden integration)
// This tests the dip transformation in isolation
void test_dip_transformation(double dip, double strike, const Vec3& msp_original,
                             const Vec3& rtx, const Vec3& rtxrx, const Vec3& rsp,
                             std::vector<double>& profile_x,
                             std::vector<double>& H_x_values,
                             std::vector<double>& H_z_values) {

    int npts = profile_x.size();

    for (int i = 0; i < npts; i++) {
        double x = profile_x[i];

        // Calculate positions
        Vec3 rtx_curr(x, 0.0, rtx.z);
        Vec3 rrx = rtx_curr - rtxrx;
        Vec3 rsp_adj(-rtx_curr.x, -rtx_curr.y, rsp.z);
        Vec3 r_rel = rrx - rsp_adj;

        // Apply dip transformation
        Vec3 msp = msp_original;

        double strike_rad = (strike - 90.0) * PI / 180.0;
        double dip_rad = (90.0 - dip) * PI / 180.0;

        Vec3 norm(std::cos(dip_rad) * std::cos(strike_rad),
                  std::sin(strike_rad) * std::cos(dip_rad),
                  std::sin(dip_rad));
        norm = norm.normalized();

        double mspdotnorm = msp.dot(norm);
        msp = norm * mspdotnorm;

        // Calculate field
        Vec3 H_sphere = static_dipole_field(msp, r_rel);

        // Apply MATLAB sign convention
        double H_tot_x = -H_sphere.x;
        double H_tot_z = H_sphere.z;

        // Convert to nT
        H_x_values[i] = (MU_0 / 1e-12) * H_tot_x;
        H_z_values[i] = (MU_0 / 1e-12) * H_tot_z;
    }
}

struct PeakInfo {
    double position;
    double value;
};

PeakInfo find_peak(const std::vector<double>& profile_x, const std::vector<double>& values) {
    int max_idx = 0;
    double max_val = std::abs(values[0]);

    for (size_t i = 1; i < values.size(); i++) {
        if (std::abs(values[i]) > max_val) {
            max_val = std::abs(values[i]);
            max_idx = i;
        }
    }

    return {profile_x[max_idx], values[max_idx]};
}

int main() {
    std::cout << std::fixed << std::setprecision(6);

    // MATLAB parameters from debug log
    double radar = 120.0;
    Vec3 rtxrx(12.0, 0.0, 56.0);
    Vec3 rsp(0.0, 0.0, -200.0);
    double strike = 90.0;

    // Create profile
    int interval = 101;
    double profile_length = 800.0;
    std::vector<double> profile_x(interval);
    double delta_x = (2.0 * profile_length) / (interval - 1);

    for (int i = 0; i < interval; i++) {
        profile_x[i] = -profile_length + i * delta_x;
    }

    // Representative induced moment (from a typical calculation at t=154.6 us)
    // This is the moment BEFORE dip transformation
    Vec3 msp_original(0.0, 0.0, -153292.796);

    Vec3 rtx(0.0, 0.0, radar);

    std::cout << "Testing Dip Angle Variation" << std::endl;
    std::cout << "============================" << std::endl << std::endl;
    std::cout << "MATLAB parameters:" << std::endl;
    std::cout << "  radar = " << radar << " m" << std::endl;
    std::cout << "  rtxrx = (" << rtxrx.x << ", " << rtxrx.y << ", " << rtxrx.z << ") m" << std::endl;
    std::cout << "  rsp = (" << rsp.x << ", " << rsp.y << ", " << rsp.z << ") m" << std::endl;
    std::cout << "  strike = " << strike << " deg" << std::endl;
    std::cout << "  msp_original = (" << msp_original.x << ", " << msp_original.y << ", " << msp_original.z << ")" << std::endl;
    std::cout << std::endl;

    // Test various dip angles
    std::vector<double> dip_angles = {0, 30, 45, 60, 85, 90, 95, 100, 120, 150, 180};

    std::cout << "Dip Angle Variation Test (Sphere contribution only)" << std::endl;
    std::cout << "====================================================" << std::endl << std::endl;

    std::cout << std::setw(8) << "Dip" << " | "
              << std::setw(12) << "X-peak pos" << " | "
              << std::setw(12) << "X-peak val" << " | "
              << std::setw(12) << "Z-peak pos" << " | "
              << std::setw(12) << "Z-peak val" << " | "
              << std::setw(12) << "Ratio |Z/X|" << std::endl;
    std::cout << std::string(90, '-') << std::endl;

    for (double dip : dip_angles) {
        std::vector<double> H_x_values(interval);
        std::vector<double> H_z_values(interval);

        test_dip_transformation(dip, strike, msp_original, rtx, rtxrx, rsp,
                               profile_x, H_x_values, H_z_values);

        PeakInfo x_peak = find_peak(profile_x, H_x_values);
        PeakInfo z_peak = find_peak(profile_x, H_z_values);

        double ratio = std::abs(z_peak.value) / std::abs(x_peak.value);

        std::cout << std::setw(8) << dip << " | "
                  << std::setw(12) << x_peak.position << " | "
                  << std::setw(12) << x_peak.value << " | "
                  << std::setw(12) << z_peak.position << " | "
                  << std::setw(12) << z_peak.value << " | "
                  << std::setw(12) << ratio << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Expected behavior: As dip varies, the ratio |Z/X| should change gradually." << std::endl;
    std::cout << "At dip=0 (horizontal), ratio should be large (vertical moment dominates)." << std::endl;
    std::cout << "At dip=90 (vertical), behavior depends on geometry." << std::endl;

    return 0;
}
