// Test how response varies with dip angle
#include <cmath>
#include <iostream>
#include <iomanip>
#include <functional>
#include <vector>

const double PI = 3.14159265358979323846;
const double MU_0 = 1.256637e-6;

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
    std::cout << std::fixed << std::setprecision(10);
    
    // Fixed parameters
    Vec3 rtxrx(12.0, 0.0, 56.0);
    Vec3 rsp(0.0, 0.0, -200.0);
    double radar = 120.0;
    double strike = 90.0;
    
    // Test at profile position 0
    double tx_x = 0.0;
    Vec3 rtx(tx_x, 0.0, radar);
    Vec3 rrx_adj(-rtxrx.x, -rtxrx.y, radar - rtxrx.z);
    Vec3 rsp_adj(-tx_x, 0.0, rsp.z);
    Vec3 r_rel = rrx_adj - rsp_adj;
    
    // Fixed induced moment (representative value from calculation)
    Vec3 msp_original(0.0, 0.0, -153292.796);
    
    std::vector<double> dip_angles = {0, 45, 85, 90, 93, 100, 110};
    
    std::cout << "Testing dip angle variation at profile position 0" << std::endl;
    std::cout << "====================================================" << std::endl << std::endl;
    std::cout << "Fixed parameters:" << std::endl;
    std::cout << "  msp_original = (0, 0, -153292.796)" << std::endl;
    std::cout << "  r_rel = (" << r_rel.x << ", " << r_rel.y << ", " << r_rel.z << ")" << std::endl;
    std::cout << "  strike = " << strike << std::endl << std::endl;
    
    std::cout << std::setw(6) << "Dip" << " | "
              << std::setw(15) << "msp_x" << " | "
              << std::setw(15) << "msp_z" << " | "
              << std::setw(15) << "H_sphere_x" << " | "
              << std::setw(15) << "H_sphere_z" << " | "
              << std::setw(15) << "H_tot_x (nT)" << " | "
              << std::setw(15) << "H_tot_z (nT)" << std::endl;
    std::cout << std::string(120, '-') << std::endl;
    
    for (double dip : dip_angles) {
        Vec3 msp = msp_original;
        
        // Apply dip transformation
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
        
        // Apply sign convention
        double H_tot_x = -H_sphere.x;
        double H_tot_z = H_sphere.z;
        
        // Convert to nT
        double H_tot_x_nT = (MU_0 / 1e-12) * H_tot_x;
        double H_tot_z_nT = (MU_0 / 1e-12) * H_tot_z;
        
        std::cout << std::setw(6) << dip << " | "
                  << std::setw(15) << msp.x << " | "
                  << std::setw(15) << msp.z << " | "
                  << std::setw(15) << H_sphere.x << " | "
                  << std::setw(15) << H_sphere.z << " | "
                  << std::setw(15) << H_tot_x_nT << " | "
                  << std::setw(15) << H_tot_z_nT << std::endl;
    }
    
    std::cout << std::endl << "Note: H_tot values shown are sphere contribution only (overburden not included)" << std::endl;
    
    return 0;
}
