// Full diagnostic test matching MATLAB exactly
#include <cmath>
#include <iostream>
#include <iomanip>
#include <functional>

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

// Static dipole field
Vec3 static_dipole_field(const Vec3& m, const Vec3& r) {
    double r2 = r.dot(r);
    if (r2 < 1e-20) return Vec3(0, 0, 0);
    double one_over_4pi = 1.0 / (4.0 * PI);
    double r_mag = std::sqrt(r2);
    double a = one_over_4pi / (r_mag * r2);
    double b = r.dot(m) * 3.0 / r2;
    return (r * b - m) * a;
}

// H_ob_x component
double H_ob_x_component(const Vec3& mtx, double dipoleM, const Vec3& rtx,
                         const Vec3& rrx, double O, double mu,
                         double sigma_ob, double thick_ob) {
    double m_x = dipoleM * mtx.x;
    double m_y = dipoleM * mtx.y;
    double m_z = dipoleM * mtx.z;
    
    double dx = rrx.x - rtx.x;
    double dy = rrx.y - rtx.y;
    
    if (rrx.z > 0) {
        double dz = rrx.z + rtx.z + (2.0 * O) / (mu * sigma_ob * thick_ob);
        double r2 = dx*dx + dy*dy + dz*dz;
        double r_5_2 = std::pow(r2, 2.5);
        double r_3_2 = std::pow(r2, 1.5);
        
        double term1 = m_x / r_3_2;
        double term2 = 3.0 * (2.0 * dx) * (m_x * dx - m_z * dz + m_y * dy) / (2.0 * r_5_2);
        
        return (-1.0 / (4.0 * PI)) * (term1 - term2);
    } else {
        double dz = rtx.z - rrx.z + (2.0 * O) / (mu * sigma_ob * thick_ob);
        double r2 = dx*dx + dy*dy + dz*dz;
        double r_5_2 = std::pow(r2, 2.5);
        double r_3_2 = std::pow(r2, 1.5);
        
        double term1 = m_x / r_3_2;
        double term2 = 3.0 * (2.0 * dx) * (m_x * dx + m_y * dy - m_z * dz) / (2.0 * r_5_2);
        
        return (-1.0 / (4.0 * PI)) * (term1 - term2);
    }
}

// H_ob_z component  
double H_ob_z_component(const Vec3& mtx, double dipoleM, const Vec3& rtx,
                         const Vec3& rrx, double O, double mu,
                         double sigma_ob, double thick_ob) {
    double m_x = dipoleM * mtx.x;
    double m_y = dipoleM * mtx.y;
    double m_z = dipoleM * mtx.z;
    
    double dx = rrx.x - rtx.x;
    double dy = rrx.y - rtx.y;
    
    if (rrx.z > 0) {
        double dz = rrx.z + rtx.z + (2.0 * O) / (mu * sigma_ob * thick_ob);
        double r2 = dx*dx + dy*dy + dz*dz;
        double r_5_2 = std::pow(r2, 2.5);
        double r_3_2 = std::pow(r2, 1.5);
        
        double term1 = m_z / r_3_2;
        double term2 = 3.0 * (2.0 * (rrx.z + rtx.z) + (4.0 * O) / (mu * sigma_ob * thick_ob)) *
                       (m_x * dx + m_z * dz + m_y * dy) / (2.0 * r_5_2);
        
        return (-1.0 / (4.0 * PI)) * (term1 - term2);
    } else {
        double dz = rtx.z - rrx.z + (2.0 * O) / (mu * sigma_ob * thick_ob);
        double r2 = dx*dx + dy*dy + dz*dz;
        double r_5_2 = std::pow(r2, 2.5);
        double r_3_2 = std::pow(r2, 1.5);
        
        double term1 = m_z / r_3_2;
        double term2 = 3.0 * (2.0 * (rtx.z - rrx.z) + (4.0 * O) / (mu * sigma_ob * thick_ob)) *
                       (m_x * dx + m_y * dy + m_z * dz) / (2.0 * r_5_2);
        
        return (-1.0 / (4.0 * PI)) * (term1 + term2);
    }
}

// dH_obdt_x
double dH_obdt_x(const Vec3& mtx, double dipoleM, const Vec3& rtx,
                 const Vec3& rrx, double O, double mu,
                 double sigma_ob, double thick_ob) {
    double m_x = dipoleM * mtx.x;
    double m_y = dipoleM * mtx.y;
    double m_z = dipoleM * mtx.z;
    
    double dx = rrx.x - rtx.x;
    double dy = rrx.y - rtx.y;
    
    if (rrx.z > 0) {
        double dz = rrx.z + rtx.z + (2.0 * O) / (mu * sigma_ob * thick_ob);
        double r2 = dx*dx + dy*dy + dz*dz;
        double r_5_2 = std::pow(r2, 2.5);
        double r_7_2 = std::pow(r2, 3.5);
        
        double tau = mu * sigma_ob * thick_ob;
        
        double term1 = m_z * (6.0 * dx) / (tau * r_5_2);
        double term2 = (6.0 * m_x * dz) / (tau * r_5_2);
        double term3 = (5.0 * (6.0 * dx) * dz * (m_x * dx - m_z * dz + m_y * dy)) / (tau * r_7_2);
        
        return (-1.0 / (4.0 * PI)) * (term1 - term2 + term3);
    } else {
        double dz = rtx.z - rrx.z + (2.0 * O) / (mu * sigma_ob * thick_ob);
        double r2 = dx*dx + dy*dy + dz*dz;
        double r_5_2 = std::pow(r2, 2.5);
        double r_7_2 = std::pow(r2, 3.5);
        
        double tau = mu * sigma_ob * thick_ob;
        
        double term1 = m_z * (6.0 * dx) / (tau * r_5_2);
        double term2 = (6.0 * m_x * dz) / (tau * r_5_2);
        double term3 = (5.0 * (6.0 * dx) * dz * (m_x * dx + m_y * dy - m_z * dz)) / (tau * r_7_2);
        
        return (-1.0 / (4.0 * PI)) * (term1 - term2 + term3);
    }
}

// dH_obdt_z
double dH_obdt_z(const Vec3& mtx, double dipoleM, const Vec3& rtx,
                 const Vec3& rrx, double O, double mu,
                 double sigma_ob, double thick_ob) {
    double m_x = dipoleM * mtx.x;
    double m_y = dipoleM * mtx.y;
    double m_z = dipoleM * mtx.z;
    
    double dx = rrx.x - rtx.x;
    double dy = rrx.y - rtx.y;
    
    if (rrx.z > 0) {
        double dz = rrx.z + rtx.z + (2.0 * O) / (mu * sigma_ob * thick_ob);
        double r2 = dx*dx + dy*dy + dz*dz;
        double r_5_2 = std::pow(r2, 2.5);
        double r_7_2 = std::pow(r2, 3.5);
        
        double tau = mu * sigma_ob * thick_ob;
        
        double term1 = (6.0 * m_z * dz) / (tau * r_5_2);
        double term2 = (6.0 * (m_x * dx + m_z * dz + m_y * dy)) / (tau * r_5_2);
        double term3 = (m_z * (6.0 * (rrx.z + rtx.z) + (12.0 * O) / tau)) / (tau * r_5_2);
        double term4 = (5.0 * (6.0 * (rrx.z + rtx.z) + (12.0 * O) / tau) * dz *
                        (m_x * dx + m_z * dz + m_y * dy)) / (tau * r_7_2);
        
        return (-1.0 / (4.0 * PI)) * (term1 - term2 + term3 + term4);
    } else {
        double dz = rtx.z - rrx.z + (2.0 * O) / (mu * sigma_ob * thick_ob);
        double r2 = dx*dx + dy*dy + dz*dz;
        double r_5_2 = std::pow(r2, 2.5);
        double r_7_2 = std::pow(r2, 3.5);
        
        double tau = mu * sigma_ob * thick_ob;
        
        double term1 = (6.0 * (m_x * dx + m_y * dy + m_z * dz)) / (tau * r_5_2);
        double term2 = (m_z * (6.0 * (rtx.z - rrx.z) + (12.0 * O) / tau)) / (tau * r_5_2);
        double term3 = (6.0 * m_z * dz) / (tau * r_5_2);
        double term4 = (5.0 * (6.0 * (rtx.z - rrx.z) + (12.0 * O) / tau) * dz *
                        (m_x * dx + m_y * dy + m_z * dz)) / (tau * r_7_2);
        
        return (-1.0 / (4.0 * PI)) * (term1 - term2 - term3 - term4);
    }
}

// Theta function
double Thetafunction_step(double t, double O, double o, double mu,
                           double sigma_sp, double a, double T) {
    double ss = mu * sigma_sp * a * a;
    double Ton2 = T / 2.0;
    double theta = 0.0;
    double temp = INFINITY;
    int k = 0;
    
    while (std::abs(theta / temp) < 1e6 && k < 10000) {
        k++;
        double k_pi_sq = (k * PI) * (k * PI);
        double exp_term = std::exp((o + O - t) * k_pi_sq / ss);
        double amp_reduction = 1.0 / (1.0 + std::exp(-Ton2 * k_pi_sq / ss));
        temp = amp_reduction * (6.0 / k_pi_sq) * exp_term;
        theta += temp;
    }
    
    return theta;
}

// Adaptive Simpson's integration
double adaptive_simpsons(std::function<double(double)> f, double a, double b,
                          double rel_tol, int max_depth, double abs_tol = 1e-20) {
    auto simpsons = [](std::function<double(double)> func, double a, double b) -> double {
        double c = (a + b) / 2.0;
        double h = (b - a) / 6.0;
        return h * (func(a) + 4.0 * func(c) + func(b));
    };
    
    std::function<double(double, double, double, int)> recursive;
    recursive = [&](double a, double b, double whole_parent, int depth) -> double {
        double c = (a + b) / 2.0;
        double whole = simpsons(f, a, b);
        double left = simpsons(f, a, c);
        double right = simpsons(f, c, b);
        
        double error = std::abs(left + right - whole);
        double tol = 15.0 * (abs_tol + rel_tol * std::abs(whole_parent));
        
        if (depth >= max_depth || error <= tol) {
            return left + right + (left + right - whole) / 15.0;
        }
        
        double new_whole = left + right;
        return recursive(a, c, new_whole, depth + 1) +
               recursive(c, b, new_whole, depth + 1);
    };
    
    double fa = f(a);
    double fb = f(b);
    double fc = f((a + b) / 2.0);
    if (std::abs(fa) < abs_tol && std::abs(fb) < abs_tol && std::abs(fc) < abs_tol) {
        return 0.0;
    }
    
    double whole_est = simpsons(f, a, b);
    return recursive(a, b, whole_est, 0);
}

// dH_tot_x_step
double dH_tot_x_step(const Vec3& mtx, double dipoleM, const Vec3& rtx,
                      const Vec3& rsp, double mu, double sigma_ob,
                      double thick_ob, double t, double o,
                      double sigma_sp, double a, double T) {
    auto fun = [&](double O) -> double {
        return -dH_obdt_x(mtx, dipoleM, rtx, rsp, O, mu, sigma_ob, thick_ob) *
                Thetafunction_step(t, O, o, mu, sigma_sp, a, T);
    };
    
    double integral_result = 0.0;
    if (t - o > 1e-10) {
        integral_result = adaptive_simpsons(fun, 0.0, t - o, 1e-5, 30, 1e-20);
    }
    
    double boundary_term = H_ob_x_component(mtx, dipoleM, rtx, rsp, -o, mu, sigma_ob, thick_ob) *
                           Thetafunction_step(t, 0.0, o, mu, sigma_sp, a, T);
    
    return integral_result + boundary_term;
}

// dH_tot_z_step
double dH_tot_z_step(const Vec3& mtx, double dipoleM, const Vec3& rtx,
                      const Vec3& rsp, double mu, double sigma_ob,
                      double thick_ob, double t, double o,
                      double sigma_sp, double a, double T) {
    auto fun = [&](double O) -> double {
        return -dH_obdt_z(mtx, dipoleM, rtx, rsp, O, mu, sigma_ob, thick_ob) *
                Thetafunction_step(t, O, o, mu, sigma_sp, a, T);
    };
    
    double integral_result = 0.0;
    if (t - o > 1e-10) {
        integral_result = adaptive_simpsons(fun, 0.0, t - o, 1e-5, 30, 1e-20);
    }
    
    double boundary_term = H_ob_z_component(mtx, dipoleM, rtx, rsp, -o, mu, sigma_ob, thick_ob) *
                           Thetafunction_step(t, 0.0, o, mu, sigma_sp, a, T);
    
    return integral_result + boundary_term;
}

int main() {
    std::cout << std::fixed << std::setprecision(15);
    
    // Exact parameters from user's MATLAB script
    Vec3 mtx(0, 0, 1);
    double dipoleM = 1.8473001e6;
    double radar = 120.0;
    Vec3 rtxrx(12.0, 0.0, 56.0);  // NOTE: 12, not 125!
    Vec3 rsp(0.0, 0.0, -200.0);
    double t = 0.0001546;  // First time window
    double mu = 1.256637e-6;
    double sigma_ob = 1.0/300.0;
    double thick_ob = 4.0;
    double sigma_sp = 0.5;
    double a = 100.0;
    bool applydip = true;
    double dip = 100.0;
    double strike = 90.0;
    double T = 1.0/30.0;
    
    std::cout << "========================================" << std::endl;
    std::cout << "C++ DIAGNOSTIC TEST" << std::endl;
    std::cout << "========================================" << std::endl << std::endl;
    std::cout << "Parameters:" << std::endl;
    std::cout << "  radar = " << radar << " m" << std::endl;
    std::cout << "  dipoleM = " << dipoleM << " A*m^2" << std::endl;
    std::cout << "  rtxrx = [" << rtxrx.x << ", " << rtxrx.y << ", " << rtxrx.z << "] m" << std::endl;
    std::cout << "  rsp = [" << rsp.x << ", " << rsp.y << ", " << rsp.z << "] m" << std::endl;
    std::cout << "  a = " << a << " m" << std::endl;
    std::cout << "  sigma_sp = " << sigma_sp << " S/m" << std::endl;
    std::cout << "  sigma_ob = " << sigma_ob << " S/m" << std::endl;
    std::cout << "  thick_ob = " << thick_ob << " m" << std::endl;
    std::cout << "  applydip = " << applydip << std::endl;
    std::cout << "  dip = " << dip << std::endl;
    std::cout << "  strike = " << strike << std::endl;
    std::cout << "  T (period) = " << T << " s" << std::endl << std::endl;
    
    // Test at profile position 0 (tx_x = 0)
    double tx_x = 0.0;
    Vec3 rtx(tx_x, 0.0, radar);
    Vec3 rrx(rtx.x - rtxrx.x, rtx.y - rtxrx.y, rtx.z - rtxrx.z);
    double profile_pos = tx_x - rtxrx.x;
    
    std::cout << "========================================" << std::endl;
    std::cout << "Profile position = " << profile_pos << " m (tx_x = " << tx_x << ")" << std::endl;
    std::cout << "========================================" << std::endl << std::endl;
    
    std::cout << "Geometry:" << std::endl;
    std::cout << "  TX position: [" << rtx.x << ", " << rtx.y << ", " << rtx.z << "]" << std::endl;
    std::cout << "  RX position: [" << rrx.x << ", " << rrx.y << ", " << rrx.z << "]" << std::endl;
    
    // Transform to calculation coordinates
    Vec3 rtx_adj(0, 0, rtx.z);
    Vec3 rsp_adj(-rtx.x, -rtx.y, rsp.z);
    Vec3 rrx_adj(-rtxrx.x, -rtxrx.y, rtx.z - rtxrx.z);
    
    std::cout << "  Transformed TX: [" << rtx_adj.x << ", " << rtx_adj.y << ", " << rtx_adj.z << "]" << std::endl;
    std::cout << "  Transformed sphere: [" << rsp_adj.x << ", " << rsp_adj.y << ", " << rsp_adj.z << "]" << std::endl;
    std::cout << "  Transformed RX: [" << rrx_adj.x << ", " << rrx_adj.y << ", " << rrx_adj.z << "]" << std::endl << std::endl;
    
    // Calculate induced moment components
    double convo_x = 2.0 * PI * a*a*a * dH_tot_x_step(mtx, dipoleM, rtx_adj, rsp_adj,
                                                         mu, sigma_ob, thick_ob, t, 0,
                                                         sigma_sp, a, T);
    double convo_y = 0.0;
    double convo_z = 2.0 * PI * a*a*a * dH_tot_z_step(mtx, dipoleM, rtx_adj, rsp_adj,
                                                         mu, sigma_ob, thick_ob, t, 0,
                                                         sigma_sp, a, T);
    
    std::cout << "Induced moment (before dip transformation):" << std::endl;
    std::cout << "  convo_x = " << convo_x << std::endl;
    std::cout << "  convo_z = " << convo_z << std::endl << std::endl;
    
    Vec3 msp(convo_x, convo_y, convo_z);
    
    // Apply dip transformation
    if (applydip) {
        std::cout << "Applying dip transformation:" << std::endl;
        std::cout << "  dip = " << dip << ", strike = " << strike << std::endl;
        
        double strike_rad = (strike - 90.0) * PI / 180.0;
        double dip_rad = (90.0 - dip) * PI / 180.0;
        
        std::cout << "  dip_rad = " << dip_rad << " rad (" << (dip_rad * 180.0 / PI) << " deg)" << std::endl;
        std::cout << "  strike_rad = " << strike_rad << " rad (" << (strike_rad * 180.0 / PI) << " deg)" << std::endl;
        
        Vec3 norm(std::cos(dip_rad) * std::cos(strike_rad),
                  std::sin(strike_rad) * std::cos(dip_rad),
                  std::sin(dip_rad));
        
        std::cout << "  norm (before normalization): (" << norm.x << ", " << norm.y << ", " << norm.z << ")" << std::endl;
        std::cout << "  |norm| = " << norm.norm() << std::endl;
        
        norm = norm.normalized();
        
        std::cout << "  norm (after normalization): (" << norm.x << ", " << norm.y << ", " << norm.z << ")" << std::endl;
        
        double mspdotnorm = msp.dot(norm);
        std::cout << "  msp.dot(norm) = " << mspdotnorm << std::endl;
        
        msp = norm * mspdotnorm;
        
        std::cout << "  msp (after projection): (" << msp.x << ", " << msp.y << ", " << msp.z << ")" << std::endl << std::endl;
    }
    
    // Calculate field from sphere
    Vec3 r_receiver = rrx_adj;
    Vec3 r_sphere = rsp_adj;
    Vec3 r_rel = r_receiver - r_sphere;
    
    std::cout << "Field calculation geometry:" << std::endl;
    std::cout << "  r_receiver = (" << r_receiver.x << ", " << r_receiver.y << ", " << r_receiver.z << ")" << std::endl;
    std::cout << "  r_sphere = (" << r_sphere.x << ", " << r_sphere.y << ", " << r_sphere.z << ")" << std::endl;
    std::cout << "  r_rel = (" << r_rel.x << ", " << r_rel.y << ", " << r_rel.z << ")" << std::endl;
    std::cout << "  |r_rel| = " << r_rel.norm() << " m" << std::endl << std::endl;
    
    Vec3 H_sphere = static_dipole_field(msp, r_rel);
    
    std::cout << "Field from sphere:" << std::endl;
    std::cout << "  H_sphere = (" << H_sphere.x << ", " << H_sphere.y << ", " << H_sphere.z << ")" << std::endl << std::endl;
    
    // Apply sign convention
    double H_tot_x = -H_sphere.x;
    double H_tot_z = H_sphere.z;
    
    // Calculate overburden contribution
    double H_obx = H_ob_x_component(mtx, dipoleM, rtx_adj, rrx_adj, t, mu, sigma_ob, thick_ob);
    double H_obz = H_ob_z_component(mtx, dipoleM, rtx_adj, rrx_adj, t, mu, sigma_ob, thick_ob);
    
    std::cout << "Overburden field:" << std::endl;
    std::cout << "  H_obx = " << H_obx << std::endl;
    std::cout << "  H_obz = " << H_obz << std::endl << std::endl;
    
    // Add overburden
    H_tot_x += H_obx;
    H_tot_z += H_obz;
    
    std::cout << "Total field (after adding overburden):" << std::endl;
    std::cout << "  H_tot_x = " << H_tot_x << std::endl;
    std::cout << "  H_tot_z = " << H_tot_z << std::endl << std::endl;
    
    // Convert to nT
    double H_tot_x_nT = (mu / 1e-12) * H_tot_x;
    double H_tot_z_nT = (mu / 1e-12) * H_tot_z;
    
    std::cout << "Scaled to nT:" << std::endl;
    std::cout << "  X = " << H_tot_x_nT << " nT" << std::endl;
    std::cout << "  Z = " << H_tot_z_nT << " nT" << std::endl;
    
    return 0;
}
