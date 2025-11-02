#include <cmath>
#include <iostream>
#include <iomanip>

const double PI = 3.14159265358979323846;

struct Vec3 {
    double x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}

    Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
    Vec3 operator*(double s) const { return Vec3(x * s, y * s, z * s); }
    double dot(const Vec3& v) const { return x * v.x + y * v.y + z * v.z; }
    double norm() const { return std::sqrt(x*x + y*y + z*z); }
    Vec3 normalized() const {
        double n = norm();
        return (n > 1e-20) ? Vec3(x/n, y/n, z/n) : Vec3();
    }
};

// Static magnetic field of a dipole
Vec3 static_dipole_field(const Vec3& m, const Vec3& r) {
    double r2 = r.dot(r);
    if (r2 < 1e-20) return Vec3(0, 0, 0);

    double one_over_4pi = 1.0 / (4.0 * PI);
    double r_mag = std::sqrt(r2);
    double a = one_over_4pi / (r_mag * r2);
    double b = r.dot(m) * 3.0 / r2;

    return (r * b - m) * a;
}

void test_with_exact_params() {
    std::cout << std::fixed << std::setprecision(10);

    // Exact parameters from user's MATLAB script
    double strike = 90.0;
    double dip = 100.0;
    bool applydip = true;

    // Sample induced moment (representative values)
    Vec3 msp_before(1.0e-5, 0.0, 5.0e-6);

    std::cout << "=== Testing with exact user parameters ===" << std::endl;
    std::cout << "dip = " << dip << ", strike = " << strike << std::endl;
    std::cout << "msp_before = (" << msp_before.x << ", " << msp_before.y << ", " << msp_before.z << ")" << std::endl;
    std::cout << std::endl;

    // C++ implementation
    Vec3 msp_cpp = msp_before;
    if (applydip) {
        double strike_rad = (strike - 90.0) * PI / 180.0;
        double dip_rad = (90.0 - dip) * PI / 180.0;

        std::cout << "C++ calculation:" << std::endl;
        std::cout << "  strike_rad = " << strike_rad << " (" << strike_rad * 180.0 / PI << " deg)" << std::endl;
        std::cout << "  dip_rad = " << dip_rad << " (" << dip_rad * 180.0 / PI << " deg)" << std::endl;

        Vec3 norm(std::cos(dip_rad) * std::cos(strike_rad),
                  std::sin(strike_rad) * std::cos(dip_rad),
                  std::sin(dip_rad));
        norm = norm.normalized();

        std::cout << "  norm (before normalization magnitude check): ("
                  << norm.x << ", " << norm.y << ", " << norm.z << ")" << std::endl;

        double mspdotnorm = msp_cpp.dot(norm);
        msp_cpp = norm * mspdotnorm;

        std::cout << "  mspdotnorm = " << mspdotnorm << std::endl;
        std::cout << "  msp_after = (" << msp_cpp.x << ", " << msp_cpp.y << ", " << msp_cpp.z << ")" << std::endl;
    }

    // MATLAB implementation
    Vec3 msp_matlab = msp_before;
    if (applydip) {
        std::cout << std::endl << "MATLAB calculation:" << std::endl;

        // norm=[cosd(90.-dip)*cosd(strike-90.),sind(strike-90.)*cosd(90.-dip),sind(90.-dip)];
        double cosd_90_minus_dip = std::cos((90.0 - dip) * PI / 180.0);
        double sind_90_minus_dip = std::sin((90.0 - dip) * PI / 180.0);
        double cosd_strike_minus_90 = std::cos((strike - 90.0) * PI / 180.0);
        double sind_strike_minus_90 = std::sin((strike - 90.0) * PI / 180.0);

        std::cout << "  cosd(90-dip) = cosd(" << (90.0-dip) << ") = " << cosd_90_minus_dip << std::endl;
        std::cout << "  sind(90-dip) = sind(" << (90.0-dip) << ") = " << sind_90_minus_dip << std::endl;
        std::cout << "  cosd(strike-90) = cosd(" << (strike-90.0) << ") = " << cosd_strike_minus_90 << std::endl;
        std::cout << "  sind(strike-90) = sind(" << (strike-90.0) << ") = " << sind_strike_minus_90 << std::endl;

        Vec3 norm(cosd_90_minus_dip * cosd_strike_minus_90,
                  sind_strike_minus_90 * cosd_90_minus_dip,
                  sind_90_minus_dip);

        std::cout << "  norm (before normalization): (" << norm.x << ", " << norm.y << ", " << norm.z << ")" << std::endl;
        std::cout << "  norm magnitude: " << norm.norm() << std::endl;

        norm = norm.normalized();

        std::cout << "  norm (after normalization): (" << norm.x << ", " << norm.y << ", " << norm.z << ")" << std::endl;

        double mspdotnorm = msp_matlab.dot(norm);
        msp_matlab = norm * mspdotnorm;

        std::cout << "  mspdotnorm = " << mspdotnorm << std::endl;
        std::cout << "  msp_after = (" << msp_matlab.x << ", " << msp_matlab.y << ", " << msp_matlab.z << ")" << std::endl;
    }

    std::cout << std::endl << "=== Comparison ===" << std::endl;
    std::cout << "Difference in msp: ("
              << (msp_cpp.x - msp_matlab.x) << ", "
              << (msp_cpp.y - msp_matlab.y) << ", "
              << (msp_cpp.z - msp_matlab.z) << ")" << std::endl;

    // Now test the field calculation with sample geometry
    Vec3 rtx(0.0, 0.0, 120.0);  // Profile position 0
    Vec3 rtxrx(12.0, 0.0, 56.0);
    Vec3 rsp(0.0, 0.0, -200.0);

    Vec3 r_receiver(-rtxrx.x, -rtxrx.y, rtx.z - rtxrx.z);
    Vec3 r_sphere(-rtx.x, -rtx.y, rsp.z);
    Vec3 r_rel = r_receiver - r_sphere;

    std::cout << std::endl << "=== Field calculation at profile position 0 ===" << std::endl;
    std::cout << "r_receiver = (" << r_receiver.x << ", " << r_receiver.y << ", " << r_receiver.z << ")" << std::endl;
    std::cout << "r_sphere = (" << r_sphere.x << ", " << r_sphere.y << ", " << r_sphere.z << ")" << std::endl;
    std::cout << "r_rel = (" << r_rel.x << ", " << r_rel.y << ", " << r_rel.z << ")" << std::endl;

    Vec3 H_sphere_cpp = static_dipole_field(msp_cpp, r_rel);
    Vec3 H_sphere_matlab = static_dipole_field(msp_matlab, r_rel);

    std::cout << std::endl << "H_sphere (C++): (" << H_sphere_cpp.x << ", " << H_sphere_cpp.y << ", " << H_sphere_cpp.z << ")" << std::endl;
    std::cout << "H_sphere (MATLAB): (" << H_sphere_matlab.x << ", " << H_sphere_matlab.y << ", " << H_sphere_matlab.z << ")" << std::endl;

    double H_tot_x_cpp = -H_sphere_cpp.x;
    double H_tot_z_cpp = H_sphere_cpp.z;
    double H_tot_x_matlab = -H_sphere_matlab.x;
    double H_tot_z_matlab = H_sphere_matlab.z;

    std::cout << std::endl << "After sign convention:" << std::endl;
    std::cout << "C++ :   H_tot_x = " << H_tot_x_cpp << ", H_tot_z = " << H_tot_z_cpp << std::endl;
    std::cout << "MATLAB: H_tot_x = " << H_tot_x_matlab << ", H_tot_z = " << H_tot_z_matlab << std::endl;
}

int main() {
    test_with_exact_params();
    return 0;
}
