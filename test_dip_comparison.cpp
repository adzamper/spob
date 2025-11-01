#include <cmath>
#include <iostream>
#include <iomanip>

const double PI = 3.14159265358979323846;

struct Vec3 {
    double x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}

    double dot(const Vec3& v) const { return x * v.x + y * v.y + z * v.z; }
    double norm() const { return std::sqrt(x*x + y*y + z*z); }
    Vec3 normalized() const {
        double n = norm();
        return (n > 1e-20) ? Vec3(x/n, y/n, z/n) : Vec3();
    }
    Vec3 operator*(double s) const { return Vec3(x * s, y * s, z * s); }
};

void test_dip_angle(double dip, double strike) {
    std::cout << std::fixed << std::setprecision(10);
    std::cout << "\n=== Testing dip=" << dip << ", strike=" << strike << " ===" << std::endl;

    // C++ calculation (current implementation)
    double strike_rad = (strike - 90.0) * PI / 180.0;
    double dip_rad = (90.0 - dip) * PI / 180.0;

    Vec3 norm_cpp(std::cos(dip_rad) * std::cos(strike_rad),
                  std::sin(strike_rad) * std::cos(dip_rad),
                  std::sin(dip_rad));
    norm_cpp = norm_cpp.normalized();

    // MATLAB calculation (for reference)
    // norm=[cosd(90.-dip)*cosd(strike-90.),sind(strike-90.)*cosd(90.-dip),sind(90.-dip)];
    double cosd_90_minus_dip = std::cos((90.0 - dip) * PI / 180.0);
    double sind_90_minus_dip = std::sin((90.0 - dip) * PI / 180.0);
    double cosd_strike_minus_90 = std::cos((strike - 90.0) * PI / 180.0);
    double sind_strike_minus_90 = std::sin((strike - 90.0) * PI / 180.0);

    Vec3 norm_matlab(cosd_90_minus_dip * cosd_strike_minus_90,
                     sind_strike_minus_90 * cosd_90_minus_dip,
                     sind_90_minus_dip);
    double norm_matlab_mag = norm_matlab.norm();
    norm_matlab = norm_matlab.normalized();

    std::cout << "C++ norm: (" << norm_cpp.x << ", " << norm_cpp.y << ", " << norm_cpp.z << ")" << std::endl;
    std::cout << "MATLAB norm: (" << norm_matlab.x << ", " << norm_matlab.y << ", " << norm_matlab.z << ")" << std::endl;
    std::cout << "Difference: (" << (norm_cpp.x - norm_matlab.x) << ", "
              << (norm_cpp.y - norm_matlab.y) << ", " << (norm_cpp.z - norm_matlab.z) << ")" << std::endl;

    // Test with a sample moment vector
    Vec3 msp_before(100.0, 0.0, 50.0);

    double dot_cpp = msp_before.dot(norm_cpp);
    Vec3 msp_after_cpp = norm_cpp * dot_cpp;

    double dot_matlab = msp_before.dot(norm_matlab);
    Vec3 msp_after_matlab = norm_matlab * dot_matlab;

    std::cout << "\nWith msp_before = (100, 0, 50):" << std::endl;
    std::cout << "C++ dot product: " << dot_cpp << std::endl;
    std::cout << "MATLAB dot product: " << dot_matlab << std::endl;
    std::cout << "C++ msp_after: (" << msp_after_cpp.x << ", " << msp_after_cpp.y << ", " << msp_after_cpp.z << ")" << std::endl;
    std::cout << "MATLAB msp_after: (" << msp_after_matlab.x << ", " << msp_after_matlab.y << ", " << msp_after_matlab.z << ")" << std::endl;
}

int main() {
    // Test cases that work
    test_dip_angle(90.0, 90.0);  // Should work
    test_dip_angle(0.0, 90.0);   // Should work

    // Test intermediate angles that reportedly don't work
    test_dip_angle(45.0, 90.0);
    test_dip_angle(85.0, 90.0);
    test_dip_angle(93.0, 90.0);
    test_dip_angle(30.0, 45.0);
    test_dip_angle(60.0, 120.0);

    return 0;
}
