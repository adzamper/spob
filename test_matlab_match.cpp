// Declare functions from sphere_overburden.cpp
#include <iostream>
#include <iomanip>
#include <cmath>

#define PI 3.14159265358979323846

struct Vec3 {
    double x, y, z;
    Vec3(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}
};

// Forward declare function
void H_total_step_1storder(const Vec3& mtx, double dipoleM, const Vec3& rtx,
                             const Vec3& rtxrx, const Vec3& rsp, double t,
                             double mu, double sigma_ob, double thick_ob,
                             double sigma_sp, double a, bool applydip,
                             double dip, double strike, double T,
                             double& H_tot_x, double& H_tot_y, double& H_tot_z,
                             double& H_x, double& H_y, double& H_z,
                             double& convo_x, double& convo_z);

int main() {
    std::cout << std::scientific << std::setprecision(10);

    // Match exact MATLAB parameters from first test case
    Vec3 mtx(0.0, 0.0, 1.0);
    double dipoleM = 1.0;
    Vec3 rtx(144.0, 0.0, 120.0);
    Vec3 rtxrx(12.0, 0.0, 56.0);
    Vec3 rsp(0.0, 0.0, -200.0);

    double t = 0.0090143;
    double T = 0.033333;
    double mu = 4.0 * PI * 1e-7;
    double sigma_ob = 0.00333333;  // MATLAB uses 0.00333333, not 0.003
    double thick_ob = 4.0;
    double sigma_sp = 0.5;
    double a = 100.0;
    double dip = 45.0;
    double strike = 90.0;
    bool applydip = true;

    std::cout << "=== MATCHING MATLAB TEST CASE ===" << std::endl;
    std::cout << "rtx = [" << rtx.x << ", " << rtx.y << ", " << rtx.z << "]" << std::endl;
    std::cout << "sigma_ob = " << sigma_ob << " (MATLAB value)" << std::endl;
    std::cout << std::endl;

    // Output variables
    double H_tot_x, H_tot_y, H_tot_z;
    double H_x, H_y, H_z;
    double convo_x, convo_z;

    // Call the function
    H_total_step_1storder(mtx, dipoleM, rtx, rtxrx, rsp, t, mu, sigma_ob,
                          thick_ob, sigma_sp, a, applydip, dip, strike, T,
                          H_tot_x, H_tot_y, H_tot_z,
                          H_x, H_y, H_z,
                          convo_x, convo_z);

    std::cout << "\nFINAL RESULTS:" << std::endl;
    std::cout << "H_tot_x = " << H_tot_x << std::endl;
    std::cout << "H_tot_z = " << H_tot_z << std::endl;
    std::cout << "convo_x = " << convo_x << std::endl;
    std::cout << "convo_z = " << convo_z << std::endl;

    std::cout << "\nEXPECTED (from MATLAB):" << std::endl;
    std::cout << "convo_x = 2.0700371320e-02" << std::endl;
    std::cout << "convo_z = 2.7578152768e-02" << std::endl;
    std::cout << "H_tot_x = -5.9762297264e-11" << std::endl;
    std::cout << "H_tot_z = 1.9399159709e-10" << std::endl;

    return 0;
}
