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

int main() {
    // Test with dip=90 (working case)
    std::cout << std::fixed << std::setprecision(10);
    std::cout << "=== Testing dip=90, strike=90 ===" << std::endl;
    {
        double dip = 90.0;
        double strike = 90.0;

        double strike_rad = (strike - 90.0) * PI / 180.0;
        double dip_rad = (90.0 - dip) * PI / 180.0;

        std::cout << "strike_rad = " << strike_rad << std::endl;
        std::cout << "dip_rad = " << dip_rad << std::endl;

        Vec3 norm(std::cos(dip_rad) * std::cos(strike_rad),
                  std::sin(strike_rad) * std::cos(dip_rad),
                  std::sin(dip_rad));

        std::cout << "norm (before normalization): (" << norm.x << ", " << norm.y << ", " << norm.z << ")" << std::endl;
        std::cout << "norm magnitude: " << norm.norm() << std::endl;

        norm = norm.normalized();
        std::cout << "norm (after normalization): (" << norm.x << ", " << norm.y << ", " << norm.z << ")" << std::endl;

        // Test projection with a sample moment
        Vec3 msp(100.0, 0.0, 50.0);
        std::cout << "msp (before dip): (" << msp.x << ", " << msp.y << ", " << msp.z << ")" << std::endl;

        double mspdotnorm = msp.dot(norm);
        std::cout << "msp.dot(norm) = " << mspdotnorm << std::endl;

        msp = norm * mspdotnorm;
        std::cout << "msp (after dip): (" << msp.x << ", " << msp.y << ", " << msp.z << ")" << std::endl;
    }

    std::cout << "\n=== Testing dip=93, strike=90 ===" << std::endl;
    {
        double dip = 93.0;
        double strike = 90.0;

        double strike_rad = (strike - 90.0) * PI / 180.0;
        double dip_rad = (90.0 - dip) * PI / 180.0;

        std::cout << "strike_rad = " << strike_rad << std::endl;
        std::cout << "dip_rad = " << dip_rad << std::endl;
        std::cout << "dip_rad in degrees = " << (dip_rad * 180.0 / PI) << std::endl;

        Vec3 norm(std::cos(dip_rad) * std::cos(strike_rad),
                  std::sin(strike_rad) * std::cos(dip_rad),
                  std::sin(dip_rad));

        std::cout << "norm (before normalization): (" << norm.x << ", " << norm.y << ", " << norm.z << ")" << std::endl;
        std::cout << "norm magnitude: " << norm.norm() << std::endl;

        norm = norm.normalized();
        std::cout << "norm (after normalization): (" << norm.x << ", " << norm.y << ", " << norm.z << ")" << std::endl;

        // Test projection with a sample moment
        Vec3 msp(100.0, 0.0, 50.0);
        std::cout << "msp (before dip): (" << msp.x << ", " << msp.y << ", " << msp.z << ")" << std::endl;

        double mspdotnorm = msp.dot(norm);
        std::cout << "msp.dot(norm) = " << mspdotnorm << std::endl;

        msp = norm * mspdotnorm;
        std::cout << "msp (after dip): (" << msp.x << ", " << msp.y << ", " << msp.z << ")" << std::endl;
    }

    return 0;
}
