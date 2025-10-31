/**
 * Sphere-Overburden Electromagnetic Response Calculator
 *
 * Based on: Desmarais and Smith, 2016. Geophysics 81(4), P. E265-E277
 * "Approximate semianalytical solutions for the electromagnetic response
 * of a dipping-sphere interacting with conductive overburden"
 *
 * This single C++ file implements the complete electromagnetic forward model
 * for compilation to WebAssembly.
 */

#include <cmath>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <functional>
#include <limits>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <emscripten/bind.h>
using namespace emscripten;
#endif

// Constants
const double PI = 3.14159265358979323846;
const double MU_0 = 1.2566370614359172e-6;  // Permeability of free space (H/m)

// ============================================================================
// 3D Vector Struct
// ============================================================================
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

// ============================================================================
// Parameters Structure
// ============================================================================
struct Parameters {
    // Survey configuration
    double radar;           // Transmitter height above ground
    double mu;              // Magnetic permeability (typically MU_0)
    double dipole_m;        // Dipole moment
    double base_freq;       // Base frequency (Hz)
    double period;          // Period (s)
    double pulse_length;    // Pulse length (s)
    double profile_length;  // Half-length of profile (m)

    // Transmitter-receiver offset
    Vec3 rtxrx;

    // Dipole moment direction (unit vector)
    Vec3 mtx;

    // Sphere parameters
    double a;               // Sphere radius (m)
    double sigma_sp;        // Sphere conductivity (S/m)
    Vec3 rsp;               // Sphere position (m)

    // Overburden parameters
    double sigma_ob;        // Overburden conductivity (S/m)
    double thick_ob;        // Overburden thickness (m)

    // Geological orientation
    bool apply_dip;         // Apply dip/strike rotation
    double strike;          // Strike angle (degrees)
    double dip;             // Dip angle (degrees)

    // Advanced options
    bool xsign_negative;    // Reverse X component sign

    // Computed values
    int interval;           // Number of profile positions

    Parameters() : mu(MU_0), interval(101) {}
};

// ============================================================================
// Time Windows (from MATLAB code)
// ============================================================================
const std::vector<double> TIME_WINDOWS = {
    0.000154600000000000,
    0.000236000000000000,
    0.000333700000000000,
    0.000447600000000000,
    0.000577800000000000,
    0.000740600000000000,
    0.000944000000000000,
    0.00118820000000000,
    0.00151370000000000,
    0.00192060000000000,
    0.00253090000000000,
    0.00334470000000000,
    0.00456540000000000,
    0.00619300000000000,
    0.00901430000000000
};

// ============================================================================
// Static Magnetic Field of a Dipole (static.m)
// ============================================================================
Vec3 static_dipole_field(const Vec3& m, const Vec3& r) {
    double r2 = r.norm2();

    if (r2 < 1e-20) {
        return Vec3(0, 0, 0);
    }

    double one_over_4pi = 1.0 / (4.0 * PI);
    double r_mag = std::sqrt(r2);
    double a = one_over_4pi / (r_mag * r2);
    double b = r.dot(m) * 3.0 / r2;

    return (r * b - m) * a;
}

// ============================================================================
// Overburden Field Components (H_ob_x.m, H_ob_z.m)
// Equations A-3a and A-3c from Desmarais & Smith 2016
// ============================================================================
double H_ob_x_component(const Vec3& mtx, double dipoleM, const Vec3& rtx,
                         const Vec3& rrx, double O, double mu,
                         double sigma_ob, double thick_ob) {
    double m_x = dipoleM * mtx.x;
    double m_y = dipoleM * mtx.y;
    double m_z = dipoleM * mtx.z;

    double dx = rrx.x - rtx.x;
    double dy = rrx.y - rtx.y;

    double H_ob_x;

    if (rrx.z > 0) {
        // Receiver above ground
        double dz = rrx.z + rtx.z + (2.0 * O) / (mu * sigma_ob * thick_ob);
        double r2 = dx*dx + dy*dy + dz*dz;
        double r_5_2 = std::pow(r2, 2.5);
        double r_3_2 = std::pow(r2, 1.5);

        double term1 = m_x / r_3_2;
        double term2 = 3.0 * (2.0 * dx) * (m_x * dx - m_z * dz + m_y * dy) / (2.0 * r_5_2);

        H_ob_x = (-1.0 / (4.0 * PI)) * (term1 - term2);
    } else {
        // Receiver below ground
        double dz = rtx.z - rrx.z + (2.0 * O) / (mu * sigma_ob * thick_ob);
        double r2 = dx*dx + dy*dy + dz*dz;
        double r_5_2 = std::pow(r2, 2.5);
        double r_3_2 = std::pow(r2, 1.5);

        double term1 = m_x / r_3_2;
        double term2 = 3.0 * (2.0 * dx) * (m_x * dx + m_y * dy - m_z * dz) / (2.0 * r_5_2);

        H_ob_x = (-1.0 / (4.0 * PI)) * (term1 - term2);
    }

    return H_ob_x;
}

double H_ob_z_component(const Vec3& mtx, double dipoleM, const Vec3& rtx,
                         const Vec3& rrx, double O, double mu,
                         double sigma_ob, double thick_ob) {
    double m_x = dipoleM * mtx.x;
    double m_y = dipoleM * mtx.y;
    double m_z = dipoleM * mtx.z;

    double dx = rrx.x - rtx.x;
    double dy = rrx.y - rtx.y;

    double H_ob_z;

    if (rrx.z > 0) {
        // Receiver above ground
        double dz = rrx.z + rtx.z + (2.0 * O) / (mu * sigma_ob * thick_ob);
        double r2 = dx*dx + dy*dy + dz*dz;
        double r_5_2 = std::pow(r2, 2.5);
        double r_3_2 = std::pow(r2, 1.5);

        double term1 = -m_z / r_3_2;
        double term2 = 3.0 * (2.0 * dz + (4.0 * O) / (mu * sigma_ob * thick_ob)) *
                       (m_x * dx - m_z * dz + m_y * dy) / (2.0 * r_5_2);

        H_ob_z = (-1.0 / (4.0 * PI)) * (term1 - term2);
    } else {
        // Receiver below ground
        double dz = rtx.z - rrx.z + (2.0 * O) / (mu * sigma_ob * thick_ob);
        double r2 = dx*dx + dy*dy + dz*dz;
        double r_5_2 = std::pow(r2, 2.5);
        double r_3_2 = std::pow(r2, 1.5);

        double term1 = m_z / r_3_2;
        double term2 = 3.0 * (2.0 * dz + (4.0 * O) / (mu * sigma_ob * thick_ob)) *
                       (m_x * dx + m_y * dy - m_z * dz) / (2.0 * r_5_2);

        H_ob_z = (-1.0 / (4.0 * PI)) * (term1 + term2);
    }

    return H_ob_z;
}

// ============================================================================
// Time Derivatives of Overburden Field (dH_obdt_x.m, dH_obdt_z.m)
// Equations A-5a and A-5c from Desmarais & Smith 2016
// ============================================================================
double dH_obdt_x(const Vec3& mtx, double dipoleM, const Vec3& rtx,
                 const Vec3& rrx, double O, double mu,
                 double sigma_ob, double thick_ob) {
    double m_x = dipoleM * mtx.x;
    double m_y = dipoleM * mtx.y;
    double m_z = dipoleM * mtx.z;

    double dx = rrx.x - rtx.x;
    double dy = rrx.y - rtx.y;

    double dH_dt;

    if (rrx.z > 0) {
        double dz = rrx.z + rtx.z + (2.0 * O) / (mu * sigma_ob * thick_ob);
        double r2 = dx*dx + dy*dy + dz*dz;
        double r_5_2 = std::pow(r2, 2.5);
        double r_7_2 = std::pow(r2, 3.5);

        double tau = mu * sigma_ob * thick_ob;

        double term1 = m_z * (6.0 * dx) / (tau * r_5_2);
        double term2 = (6.0 * m_x * dz) / (tau * r_5_2);
        double term3 = (5.0 * (6.0 * dx) * dz * (m_x * dx - m_z * dz + m_y * dy)) / (tau * r_7_2);

        dH_dt = (-1.0 / (4.0 * PI)) * (term1 - term2 + term3);
    } else {
        double dz = rtx.z - rrx.z + (2.0 * O) / (mu * sigma_ob * thick_ob);
        double r2 = dx*dx + dy*dy + dz*dz;
        double r_5_2 = std::pow(r2, 2.5);
        double r_7_2 = std::pow(r2, 3.5);

        double tau = mu * sigma_ob * thick_ob;

        double term1 = m_z * (6.0 * dx) / (tau * r_5_2);
        double term2 = (6.0 * m_x * dz) / (tau * r_5_2);
        double term3 = (5.0 * (6.0 * dx) * dz * (m_x * dx + m_y * dy - m_z * dz)) / (tau * r_7_2);

        dH_dt = (-1.0 / (4.0 * PI)) * (term1 - term2 + term3);
    }

    return dH_dt;
}

double dH_obdt_z(const Vec3& mtx, double dipoleM, const Vec3& rtx,
                 const Vec3& rrx, double O, double mu,
                 double sigma_ob, double thick_ob) {
    double m_x = dipoleM * mtx.x;
    double m_y = dipoleM * mtx.y;
    double m_z = dipoleM * mtx.z;

    double dx = rrx.x - rtx.x;
    double dy = rrx.y - rtx.y;

    double dH_dt;

    if (rrx.z > 0) {
        double dz = rrx.z + rtx.z + (2.0 * O) / (mu * sigma_ob * thick_ob);
        double r2 = dx*dx + dy*dy + dz*dz;
        double r_5_2 = std::pow(r2, 2.5);
        double r_7_2 = std::pow(r2, 3.5);

        double tau = mu * sigma_ob * thick_ob;

        double term1 = (6.0 * m_z * dz) / (tau * r_5_2);
        double term2 = (6.0 * (m_x * dx - m_z * dz + m_y * dy)) / (tau * r_5_2);
        double term3 = (m_z * (6.0 * dz + (12.0 * O) / tau)) / (tau * r_5_2);
        double term4 = (5.0 * (6.0 * dz + (12.0 * O) / tau) * dz *
                        (m_x * dx - m_z * dz + m_y * dy)) / (tau * r_7_2);

        dH_dt = (-1.0 / (4.0 * PI)) * (term1 - term2 + term3 + term4);
    } else {
        double dz = rtx.z - rrx.z + (2.0 * O) / (mu * sigma_ob * thick_ob);
        double r2 = dx*dx + dy*dy + dz*dz;
        double r_5_2 = std::pow(r2, 2.5);
        double r_7_2 = std::pow(r2, 3.5);

        double tau = mu * sigma_ob * thick_ob;

        double term1 = (6.0 * (m_x * dx + m_y * dy - m_z * dz)) / (tau * r_5_2);
        double term2 = (m_z * (6.0 * dz + (12.0 * O) / tau)) / (tau * r_5_2);
        double term3 = (6.0 * m_z * dz) / (tau * r_5_2);
        double term4 = (5.0 * (6.0 * dz + (12.0 * O) / tau) * dz *
                        (m_x * dx + m_y * dy - m_z * dz)) / (tau * r_7_2);

        dH_dt = (-1.0 / (4.0 * PI)) * (term1 - term2 - term3 - term4);
    }

    return dH_dt;
}

// ============================================================================
// Theta Function - Time-dependent part of sphere step response
// Equations 12-13 and 21b from Desmarais & Smith 2016
// ============================================================================
double Thetafunction_step(double t, double O, double o, double mu,
                           double sigma_sp, double a, double T) {
    double ss = mu * sigma_sp * a * a;
    double Ton2 = T / 2.0;
    double theta = 0.0;
    double temp = INFINITY;
    int k = 0;

    // Sum until convergence: theta/temp >= 1e6 (term < 1 ppm of sum)
    // Matches MATLAB: while (theta/temp) < 1E6
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

// ============================================================================
// Adaptive Simpson's Integration with separate absolute and relative tolerance
// Matches MATLAB's integral() function behavior
// ============================================================================
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

        // Combined absolute and relative tolerance (MATLAB-style)
        // tol = 15 * (AbsTol + RelTol * |integral_estimate|)
        double tol = 15.0 * (abs_tol + rel_tol * std::abs(whole_parent));

        if (depth >= max_depth || error <= tol) {
            return left + right + (left + right - whole) / 15.0;
        }

        double new_whole = left + right;
        return recursive(a, c, new_whole, depth + 1) +
               recursive(c, b, new_whole, depth + 1);
    };

    // Handle case where integrand might be zero or very small everywhere
    double fa = f(a);
    double fb = f(b);
    double fc = f((a + b) / 2.0);
    if (std::abs(fa) < abs_tol && std::abs(fb) < abs_tol && std::abs(fc) < abs_tol) {
        return 0.0;
    }

    double whole_est = simpsons(f, a, b);
    return recursive(a, b, whole_est, 0);
}

// ============================================================================
// First-Order Induced Moment Calculation (dH_tot_x_step.m, dH_tot_z_step.m)
// Equation 16b from Desmarais & Smith 2016
// ============================================================================
double dH_tot_x_step(const Vec3& mtx, double dipoleM, const Vec3& rtx,
                      const Vec3& rsp, double mu, double sigma_ob,
                      double thick_ob, double t, double o,
                      double sigma_sp, double a, double T) {
    // Define integrand
    auto fun = [&](double O) -> double {
        return -dH_obdt_x(mtx, dipoleM, rtx, rsp, O, mu, sigma_ob, thick_ob) *
                Thetafunction_step(t, O, o, mu, sigma_sp, a, T);
    };

    // Compute integral from 0 to t-o with adaptive quadrature
    // MATLAB: integral(fun, 0, t-o, 'RelTol', 1e-5, 'AbsTol', 1e-20)
    double integral_result = 0.0;
    if (t - o > 1e-10) {
        integral_result = adaptive_simpsons(fun, 0.0, t - o, 1e-5, 30, 1e-20);
    }

    // Add the boundary term
    double boundary_term = H_ob_x_component(mtx, dipoleM, rtx, rsp, -o, mu, sigma_ob, thick_ob) *
                           Thetafunction_step(t, 0.0, o, mu, sigma_sp, a, T);

    return integral_result + boundary_term;
}

double dH_tot_z_step(const Vec3& mtx, double dipoleM, const Vec3& rtx,
                      const Vec3& rsp, double mu, double sigma_ob,
                      double thick_ob, double t, double o,
                      double sigma_sp, double a, double T) {
    // Define integrand
    auto fun = [&](double O) -> double {
        return -dH_obdt_z(mtx, dipoleM, rtx, rsp, O, mu, sigma_ob, thick_ob) *
                Thetafunction_step(t, O, o, mu, sigma_sp, a, T);
    };

    // Compute integral from 0 to t-o with adaptive quadrature
    // MATLAB: integral(fun, 0, t-o, 'RelTol', 1e-5, 'AbsTol', 1e-20)
    double integral_result = 0.0;
    if (t - o > 1e-10) {
        integral_result = adaptive_simpsons(fun, 0.0, t - o, 1e-5, 30, 1e-20);
    }

    // Add the boundary term
    double boundary_term = H_ob_z_component(mtx, dipoleM, rtx, rsp, -o, mu, sigma_ob, thick_ob) *
                           Thetafunction_step(t, 0.0, o, mu, sigma_sp, a, T);

    return integral_result + boundary_term;
}

// ============================================================================
// Total Field Calculation (H_total_step_1storder.m)
// First-order approximation of sphere-overburden system
// ============================================================================
void H_total_step_1storder(const Vec3& mtx, double dipoleM, const Vec3& rtx,
                             const Vec3& rtxrx, const Vec3& rsp, double t,
                             double mu, double sigma_ob, double thick_ob,
                             double sigma_sp, double a, bool applydip,
                             double dip, double strike, double T,
                             double& H_tot_x, double& H_tot_y, double& H_tot_z,
                             double& H_x, double& H_y, double& H_z,
                             double& convo_x, double& convo_z) {
    // Calculate first-order induced moment components
    Vec3 rtx_adj(0, 0, rtx.z);
    Vec3 rsp_adj(-rtx.x, -rtx.y, rsp.z);

    convo_x = 2.0 * PI * a*a*a * dH_tot_x_step(mtx, dipoleM, rtx_adj, rsp_adj,
                                                  mu, sigma_ob, thick_ob, t, 0,
                                                  sigma_sp, a, T);
    double convo_y = 0.0;  // Y component not implemented
    convo_z = 2.0 * PI * a*a*a * dH_tot_z_step(mtx, dipoleM, rtx_adj, rsp_adj,
                                                  mu, sigma_ob, thick_ob, t, 0,
                                                  sigma_sp, a, T);

    // Store sphere moment
    Vec3 msp(convo_x, convo_y, convo_z);

    // Apply dipping sphere model if requested
    if (applydip) {
        // Calculate normal vector to dipping plane
        double strike_rad = (strike - 90.0) * PI / 180.0;
        double dip_rad = (90.0 - dip) * PI / 180.0;

        Vec3 norm(std::cos(dip_rad) * std::cos(strike_rad),
                  std::sin(strike_rad) * std::cos(dip_rad),
                  std::sin(dip_rad));

        // Normalize
        norm = norm.normalized();

        // Project moment onto normal direction
        double mspdotnorm = msp.dot(norm);
        msp = norm * mspdotnorm;
    }

    // Calculate field using induced moment
    Vec3 r_receiver = Vec3(-rtxrx.x, -rtxrx.y, rtx.z - rtxrx.z);
    Vec3 r_sphere = Vec3(-rtx.x, -rtx.y, rsp.z);
    Vec3 r_rel = r_receiver - r_sphere;

    Vec3 H_sphere = static_dipole_field(msp, r_rel);

    H_tot_x = -H_sphere.x;  // Note: negative sign for x component
    H_tot_z = H_sphere.z;

    // Calculate 0th order term (overburden alone)
    Vec3 rrx_ob(-rtxrx.x, -rtxrx.y, rtx.z - rtxrx.z);
    H_x = H_ob_x_component(mtx, dipoleM, rtx_adj, rrx_ob, t, mu, sigma_ob, thick_ob);
    H_z = H_ob_z_component(mtx, dipoleM, rtx_adj, rrx_ob, t, mu, sigma_ob, thick_ob);

    // Add overburden response
    H_tot_x += H_x;
    H_tot_z += H_z;

    // Y components are zero (not implemented)
    H_tot_y = 0.0;
    H_y = 0.0;
}

// ============================================================================
// Main Calculation Function
// ============================================================================
std::string calculate_em_response(const std::string& params_json) {
    Parameters params;

    // Set defaults first
    params.radar = 120.0;
    params.mu = MU_0;
    params.dipole_m = 1.847300e6;
    params.base_freq = 30.0;
    params.period = 1.0 / params.base_freq;
    params.pulse_length = 3.65e-3;
    params.profile_length = 800.0;
    params.rtxrx = Vec3(125.0, 0.0, 56.0);
    params.mtx = Vec3(0.0, 0.0, 1.0);
    params.a = 100.0;
    params.sigma_sp = 0.5;
    params.rsp = Vec3(0.0, 0.0, -200.0);
    params.sigma_ob = 1.0 / 30.0;
    params.thick_ob = 4.0;
    params.apply_dip = false;
    params.strike = 90.0;
    params.dip = 85.0;
    params.xsign_negative = false;
    params.interval = 101;

    // Parse JSON - simple manual parsing (no external library needed)
    if (!params_json.empty() && params_json != "{}") {
        auto extract_num = [](const std::string& json, const std::string& key) -> double {
            size_t pos = json.find("\"" + key + "\"");
            if (pos == std::string::npos) return std::numeric_limits<double>::quiet_NaN();
            pos = json.find(":", pos);
            if (pos == std::string::npos) return std::numeric_limits<double>::quiet_NaN();
            size_t end = json.find_first_of(",}", pos);
            std::string val_str = json.substr(pos + 1, end - pos - 1);
            try { return std::stod(val_str); } catch (...) { return std::numeric_limits<double>::quiet_NaN(); }
        };

        auto extract_bool = [](const std::string& json, const std::string& key) -> bool {
            size_t pos = json.find("\"" + key + "\"");
            if (pos == std::string::npos) return false;
            pos = json.find(":", pos);
            return json.find("true", pos) < json.find(",", pos) ||
                   json.find("true", pos) < json.find("}", pos);
        };

        double val;

        // Survey parameters
        if (!std::isnan(val = extract_num(params_json, "radar"))) params.radar = val;
        if (!std::isnan(val = extract_num(params_json, "dipole_m"))) params.dipole_m = val;
        if (!std::isnan(val = extract_num(params_json, "base_freq"))) params.base_freq = val;
        if (!std::isnan(val = extract_num(params_json, "pulse_length"))) params.pulse_length = val;
        if (!std::isnan(val = extract_num(params_json, "period"))) params.period = val;
        if (!std::isnan(val = extract_num(params_json, "profile_length"))) params.profile_length = val;

        // Tx-Rx offset vector
        if (!std::isnan(val = extract_num(params_json, "rtxrx_x"))) params.rtxrx.x = val;
        if (!std::isnan(val = extract_num(params_json, "rtxrx_y"))) params.rtxrx.y = val;
        if (!std::isnan(val = extract_num(params_json, "rtxrx_z"))) params.rtxrx.z = val;

        // Dipole direction vector
        if (!std::isnan(val = extract_num(params_json, "mtx_x"))) params.mtx.x = val;
        if (!std::isnan(val = extract_num(params_json, "mtx_y"))) params.mtx.y = val;
        if (!std::isnan(val = extract_num(params_json, "mtx_z"))) params.mtx.z = val;
        params.mtx = params.mtx.normalized();

        // Sphere parameters
        if (!std::isnan(val = extract_num(params_json, "a"))) params.a = val;
        if (!std::isnan(val = extract_num(params_json, "sigma_sp"))) params.sigma_sp = val;
        if (!std::isnan(val = extract_num(params_json, "rsp_x"))) params.rsp.x = val;
        if (!std::isnan(val = extract_num(params_json, "rsp_y"))) params.rsp.y = val;
        if (!std::isnan(val = extract_num(params_json, "rsp_z"))) params.rsp.z = val;

        // Overburden parameters
        if (!std::isnan(val = extract_num(params_json, "sigma_ob"))) params.sigma_ob = val;
        if (!std::isnan(val = extract_num(params_json, "thick_ob"))) params.thick_ob = val;

        // Geology parameters
        params.apply_dip = extract_bool(params_json, "apply_dip");
        if (!std::isnan(val = extract_num(params_json, "strike"))) params.strike = val;
        if (!std::isnan(val = extract_num(params_json, "dip"))) params.dip = val;

        // Advanced parameters
        params.xsign_negative = extract_bool(params_json, "xsign_negative");
    }

    // Allocate result arrays
    int nw = TIME_WINDOWS.size();
    int interval = params.interval;

    std::vector<std::vector<double>> H_tot_x(nw, std::vector<double>(interval));
    std::vector<std::vector<double>> H_tot_y(nw, std::vector<double>(interval));
    std::vector<std::vector<double>> H_tot_z(nw, std::vector<double>(interval));
    std::vector<double> profile(interval);

    double delta_x = (2.0 * params.profile_length) / (interval - 1);

    // Main calculation loop
    for (int j = 0; j < nw; j++) {
        double wc = TIME_WINDOWS[j];

        for (int i = 0; i < interval; i++) {
            double x = -params.profile_length + i * delta_x;
            profile[i] = x - params.rtxrx.x;

            Vec3 rtx(x, 0.0, params.radar);
            Vec3 rrx(rtx.x - params.rtxrx.x, rtx.y - params.rtxrx.y, rtx.z - params.rtxrx.z);

            double H_x, H_y, H_z;
            double H_obx, H_oby, H_obz;
            double convo_x, convo_z;

            H_total_step_1storder(params.mtx, params.dipole_m, rtx, params.rtxrx,
                                   params.rsp, wc, params.mu, params.sigma_ob,
                                   params.thick_ob, params.sigma_sp, params.a,
                                   params.apply_dip, params.dip, params.strike,
                                   params.period,
                                   H_x, H_y, H_z, H_obx, H_oby, H_obz,
                                   convo_x, convo_z);

            // Convert to nT (multiply by mu and scale)
            H_tot_x[j][i] = (params.mu / 1e-12) * H_x * (params.xsign_negative ? -1.0 : 1.0);
            H_tot_y[j][i] = (params.mu / 1e-12) * H_y;
            H_tot_z[j][i] = (params.mu / 1e-12) * H_z;
        }
    }

    // Build JSON response
    std::ostringstream json_out;
    json_out << "{";
    json_out << "\"x_values\":[";
    for (int i = 0; i < interval; i++) {
        if (i > 0) json_out << ",";
        json_out << profile[i];
    }
    json_out << "],";

    json_out << "\"time_windows\":[";
    for (size_t j = 0; j < TIME_WINDOWS.size(); j++) {
        if (j > 0) json_out << ",";
        json_out << TIME_WINDOWS[j];
    }
    json_out << "],";

    json_out << "\"x_components\":[";
    for (int j = 0; j < nw; j++) {
        if (j > 0) json_out << ",";
        json_out << "[";
        for (int i = 0; i < interval; i++) {
            if (i > 0) json_out << ",";
            json_out << H_tot_x[j][i];
        }
        json_out << "]";
    }
    json_out << "],";

    json_out << "\"y_components\":[";
    for (int j = 0; j < nw; j++) {
        if (j > 0) json_out << ",";
        json_out << "[";
        for (int i = 0; i < interval; i++) {
            if (i > 0) json_out << ",";
            json_out << H_tot_y[j][i];
        }
        json_out << "]";
    }
    json_out << "],";

    json_out << "\"z_components\":[";
    for (int j = 0; j < nw; j++) {
        if (j > 0) json_out << ",";
        json_out << "[";
        for (int i = 0; i < interval; i++) {
            if (i > 0) json_out << ",";
            json_out << H_tot_z[j][i];
        }
        json_out << "]";
    }
    json_out << "]";

    json_out << "}";

    return json_out.str();
}

// ============================================================================
// Emscripten Bindings
// ============================================================================
#ifdef __EMSCRIPTEN__
EMSCRIPTEN_BINDINGS(sphere_overburden) {
    function("calculate_em_response", &calculate_em_response);
}
#endif

// ============================================================================
// Main function for testing/standalone compilation
// ============================================================================
#ifndef __EMSCRIPTEN__
int main() {
    std::string result = calculate_em_response("{}");
    // In standalone mode, you could print or process the result
    return 0;
}
#endif
