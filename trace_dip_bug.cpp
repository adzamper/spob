// Trace through the calculation at x=-76m for dip=90 and dip=92
// to understand why the peak shifts

#include <cmath>
#include <iostream>
#include <iomanip>

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

void trace_calculation(double tx_x, double dip, double strike) {
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "\n======================================\n";
    std::cout << "TX_x = " << tx_x << " m, dip = " << dip << "°\n";
    std::cout << "======================================\n\n";

    // Parameters
    double rtxrx_x = 12.0;
    double rtxrx_z = 56.0;
    Vec3 rtxrx(rtxrx_x, 0.0, rtxrx_z);
    Vec3 rsp(0.0, 0.0, -200.0);
    double radar = 120.0;

    // TX and RX positions
    Vec3 rtx(tx_x, 0.0, radar);
    Vec3 rrx(rtx.x - rtxrx.x, rtx.y - rtxrx.y, rtx.z - rtxrx.z);

    std::cout << "Geometry:\n";
    std::cout << "  TX position: (" << rtx.x << ", " << rtx.y << ", " << rtx.z << ")\n";
    std::cout << "  RX position: (" << rrx.x << ", " << rrx.y << ", " << rrx.z << ")\n";
    std::cout << "  Sphere position: (" << rsp.x << ", " << rsp.y << ", " << rsp.z << ")\n\n";

    // Profile position
    double profile_pos = tx_x - rtxrx_x;
    std::cout << "Profile position: " << profile_pos << " m\n\n";

    // Simplified induced moment (using representative values from full calculation)
    // For this trace, I'll use typical values. In reality these come from dH_tot_x_step/dH_tot_z_step
    double convo_x = 5000.0 * (tx_x / 100.0);  // Simplified - scales with tx position
    double convo_z = -150000.0;  // Typical value for vertical dipole

    std::cout << "Induced moment (before dip transformation):\n";
    std::cout << "  convo_x = " << convo_x << "\n";
    std::cout << "  convo_z = " << convo_z << "\n\n";

    Vec3 msp(convo_x, 0.0, convo_z);

    // Apply dip transformation
    double strike_rad = (strike - 90.0) * PI / 180.0;
    double dip_rad = (90.0 - dip) * PI / 180.0;

    std::cout << "Dip transformation:\n";
    std::cout << "  strike = " << strike << "° -> strike_rad = " << strike_rad << " rad\n";
    std::cout << "  dip = " << dip << "° -> dip_rad = (90-" << dip << ")° = ";
    std::cout << (90.0 - dip) << "° = " << dip_rad << " rad\n\n";

    Vec3 norm(std::cos(dip_rad) * std::cos(strike_rad),
              std::sin(strike_rad) * std::cos(dip_rad),
              std::sin(dip_rad));

    std::cout << "Normal vector (before normalization):\n";
    std::cout << "  norm = (" << norm.x << ", " << norm.y << ", " << norm.z << ")\n";
    std::cout << "  |norm| = " << norm.norm() << "\n\n";

    norm = norm.normalized();

    std::cout << "Normal vector (after normalization):\n";
    std::cout << "  norm = (" << norm.x << ", " << norm.y << ", " << norm.z << ")\n\n";

    // Project moment
    double mspdotnorm = msp.dot(norm);
    std::cout << "Projection:\n";
    std::cout << "  msp · norm = " << mspdotnorm << "\n";

    Vec3 msp_projected = norm * mspdotnorm;
    std::cout << "  msp_projected = (" << msp_projected.x << ", " << msp_projected.y;
    std::cout << ", " << msp_projected.z << ")\n\n";

    // Calculate field at receiver
    Vec3 r_receiver(-rtxrx.x, -rtxrx.y, rtx.z - rtxrx.z);
    Vec3 r_sphere(-rtx.x, -rtx.y, rsp.z);
    Vec3 r_rel = r_receiver - r_sphere;

    std::cout << "Field calculation:\n";
    std::cout << "  r_receiver = (" << r_receiver.x << ", " << r_receiver.y << ", " << r_receiver.z << ")\n";
    std::cout << "  r_sphere = (" << r_sphere.x << ", " << r_sphere.y << ", " << r_sphere.z << ")\n";
    std::cout << "  r_rel = (" << r_rel.x << ", " << r_rel.y << ", " << r_rel.z << ")\n";
    std::cout << "  |r_rel| = " << r_rel.norm() << " m\n\n";

    Vec3 H_sphere = static_dipole_field(msp_projected, r_rel);

    double H_tot_x = -H_sphere.x;
    double H_tot_z = H_sphere.z;

    double H_tot_x_nT = (MU_0 / 1e-12) * H_tot_x;
    double H_tot_z_nT = (MU_0 / 1e-12) * H_tot_z;

    std::cout << "Field from sphere:\n";
    std::cout << "  H_sphere = (" << H_sphere.x << ", " << H_sphere.y << ", " << H_sphere.z << ")\n";
    std::cout << "  H_tot_x = " << H_tot_x << " (before nT conversion)\n";
    std::cout << "  H_tot_z = " << H_tot_z << " (before nT conversion)\n\n";

    std::cout << "Final result (sphere only, no overburden):\n";
    std::cout << "  H_tot_x = " << H_tot_x_nT << " nT\n";
    std::cout << "  H_tot_z = " << H_tot_z_nT << " nT\n";
}

int main() {
    std::cout << "TRACING DIP BUG: Why does peak position shift with dip?\n";
    std::cout << "=======================================================\n";

    // Trace at the position where peak occurs for dip=90
    double tx_x_for_peak = -64.0;  // Profile -76m = TX_x -76+12 = -64m

    trace_calculation(tx_x_for_peak, 90.0, 90.0);
    trace_calculation(tx_x_for_peak, 92.0, 90.0);

    std::cout << "\n\nKEY INSIGHT:\n";
    std::cout << "============\n";
    std::cout << "When dip changes from 90° to 92°, the normal vector changes:\n";
    std::cout << "  dip=90: norm = (1, 0, 0)\n";
    std::cout << "  dip=92: norm = (0.9994, 0, -0.0349)\n\n";
    std::cout << "This changes the projected moment direction, which changes\n";
    std::cout << "the dipole field pattern, causing peaks to shift!\n\n";
    std::cout << "QUESTION: Is the dip formula (90-dip) correct for dip>90?\n";

    return 0;
}
