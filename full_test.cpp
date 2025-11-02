// Minimal test to check dip=100 with exact user parameters
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

Vec3 static_dipole_field(const Vec3& m, const Vec3& r) {
    double r2 = r.dot(r);
    if (r2 < 1e-20) return Vec3(0, 0, 0);
    double one_over_4pi = 1.0 / (4.0 * PI);
    double r_mag = std::sqrt(r2);
    double a = one_over_4pi / (r_mag * r2);
    double b = r.dot(m) * 3.0 / r2;
    return (r * b - m) * a;
}

int main() {
    std::cout << std::fixed << std::setprecision(15);

    // Exact parameters from user's MATLAB
    double dip = 100.0;
    double strike = 90.0;
    bool applydip = true;

    // Sample moment values (these would come from integration in real code)
    // Using representative values
    Vec3 msp_initial(1.0e-5, 0.0, 5.0e-6);

    std::cout << "=== MATLAB Calculation ===" << std::endl;
    Vec3 msp_matlab = msp_initial;
    
    // MATLAB: norm=[cosd(90.-dip)*cosd(strike-90.),sind(strike-90.)*cosd(90.-dip),sind(90.-dip)];
    double cosd_90_minus_dip = std::cos((90.0 - dip) * PI / 180.0);
    double sind_90_minus_dip = std::sin((90.0 - dip) * PI / 180.0);
    double cosd_strike_minus_90 = std::cos((strike - 90.0) * PI / 180.0);
    double sind_strike_minus_90 = std::sin((strike - 90.0) * PI / 180.0);

    std::cout << "Angle calculations:" << std::endl;
    std::cout << "  90 - dip = " << (90.0 - dip) << " degrees" << std::endl;
    std::cout << "  strike - 90 = " << (strike - 90.0) << " degrees" << std::endl;
    std::cout << "  cosd(90-dip) = " << cosd_90_minus_dip << std::endl;
    std::cout << "  sind(90-dip) = " << sind_90_minus_dip << std::endl;
    std::cout << "  cosd(strike-90) = " << cosd_strike_minus_90 << std::endl;
    std::cout << "  sind(strike-90) = " << sind_strike_minus_90 << std::endl;

    Vec3 norm_matlab(cosd_90_minus_dip * cosd_strike_minus_90,
                     sind_strike_minus_90 * cosd_90_minus_dip,
                     sind_90_minus_dip);

    std::cout << "\nNormal vector:" << std::endl;
    std::cout << "  Before normalization: (" << norm_matlab.x << ", " << norm_matlab.y << ", " << norm_matlab.z << ")" << std::endl;
    std::cout << "  Magnitude: " << norm_matlab.norm() << std::endl;
    norm_matlab = norm_matlab.normalized();
    std::cout << "  After normalization: (" << norm_matlab.x << ", " << norm_matlab.y << ", " << norm_matlab.z << ")" << std::endl;

    double mspdotnorm_matlab = msp_matlab.dot(norm_matlab);
    msp_matlab = norm_matlab * mspdotnorm_matlab;

    std::cout << "\nMoment projection:" << std::endl;
    std::cout << "  msp_initial = (" << msp_initial.x << ", " << msp_initial.y << ", " << msp_initial.z << ")" << std::endl;
    std::cout << "  msp.dot(norm) = " << mspdotnorm_matlab << std::endl;
    std::cout << "  msp_projected = (" << msp_matlab.x << ", " << msp_matlab.y << ", " << msp_matlab.z << ")" << std::endl;

    std::cout << "\n=== C++ Calculation ===" << std::endl;
    Vec3 msp_cpp = msp_initial;
    
    double strike_rad = (strike - 90.0) * PI / 180.0;
    double dip_rad = (90.0 - dip) * PI / 180.0;

    std::cout << "Angle calculations:" << std::endl;
    std::cout << "  dip_rad = " << dip_rad << " radians = " << (dip_rad * 180.0 / PI) << " degrees" << std::endl;
    std::cout << "  strike_rad = " << strike_rad << " radians = " << (strike_rad * 180.0 / PI) << " degrees" << std::endl;

    Vec3 norm_cpp(std::cos(dip_rad) * std::cos(strike_rad),
                  std::sin(strike_rad) * std::cos(dip_rad),
                  std::sin(dip_rad));

    std::cout << "\nNormal vector:" << std::endl;
    std::cout << "  Before normalization: (" << norm_cpp.x << ", " << norm_cpp.y << ", " << norm_cpp.z << ")" << std::endl;
    std::cout << "  Magnitude: " << norm_cpp.norm() << std::endl;
    norm_cpp = norm_cpp.normalized();
    std::cout << "  After normalization: (" << norm_cpp.x << ", " << norm_cpp.y << ", " << norm_cpp.z << ")" << std::endl;

    double mspdotnorm_cpp = msp_cpp.dot(norm_cpp);
    msp_cpp = norm_cpp * mspdotnorm_cpp;

    std::cout << "\nMoment projection:" << std::endl;
    std::cout << "  msp_initial = (" << msp_initial.x << ", " << msp_initial.y << ", " << msp_initial.z << ")" << std::endl;
    std::cout << "  msp.dot(norm) = " << mspdotnorm_cpp << std::endl;
    std::cout << "  msp_projected = (" << msp_cpp.x << ", " << msp_cpp.y << ", " << msp_cpp.z << ")" << std::endl;

    std::cout << "\n=== Comparison ===" << std::endl;
    std::cout << "Difference in projected moment:" << std::endl;
    std::cout << "  Δx = " << (msp_cpp.x - msp_matlab.x) << std::endl;
    std::cout << "  Δy = " << (msp_cpp.y - msp_matlab.y) << std::endl;
    std::cout << "  Δz = " << (msp_cpp.z - msp_matlab.z) << std::endl;

    std::cout << "\n=== Field Calculation Test ===" << std::endl;
    // Test geometry
    Vec3 rtx(0.0, 0.0, 120.0);
    Vec3 rtxrx(12.0, 0.0, 56.0);
    Vec3 rsp(0.0, 0.0, -200.0);

    Vec3 r_receiver(-rtxrx.x, -rtxrx.y, rtx.z - rtxrx.z);
    Vec3 r_sphere(-rtx.x, -rtx.y, rsp.z);
    Vec3 r_rel = r_receiver - r_sphere;

    std::cout << "Geometry:" << std::endl;
    std::cout << "  r_receiver = (" << r_receiver.x << ", " << r_receiver.y << ", " << r_receiver.z << ")" << std::endl;
    std::cout << "  r_sphere = (" << r_sphere.x << ", " << r_sphere.y << ", " << r_sphere.z << ")" << std::endl;
    std::cout << "  r_rel = (" << r_rel.x << ", " << r_rel.y << ", " << r_rel.z << ")" << std::endl;
    std::cout << "  |r_rel| = " << r_rel.norm() << " m" << std::endl;

    Vec3 H_matlab = static_dipole_field(msp_matlab, r_rel);
    Vec3 H_cpp = static_dipole_field(msp_cpp, r_rel);

    std::cout << "\nField from sphere:" << std::endl;
    std::cout << "  MATLAB: (" << H_matlab.x << ", " << H_matlab.y << ", " << H_matlab.z << ")" << std::endl;
    std::cout << "  C++:    (" << H_cpp.x << ", " << H_cpp.y << ", " << H_cpp.z << ")" << std::endl;

    double H_tot_x_matlab = -H_matlab.x;
    double H_tot_z_matlab = H_matlab.z;
    double H_tot_x_cpp = -H_cpp.x;
    double H_tot_z_cpp = H_cpp.z;

    std::cout << "\nAfter sign convention:" << std::endl;
    std::cout << "  MATLAB: H_tot_x = " << H_tot_x_matlab << ", H_tot_z = " << H_tot_z_matlab << std::endl;
    std::cout << "  C++:    H_tot_x = " << H_tot_x_cpp << ", H_tot_z = " << H_tot_z_cpp << std::endl;

    return 0;
}
