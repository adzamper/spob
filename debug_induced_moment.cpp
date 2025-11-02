// Debug: Print induced moment values at different positions
#define LIBRARY_BUILD
#define DEBUG_INDUCED_MOMENT
#include "sphere_overburden.cpp"
#include <iostream>
#include <iomanip>

int main() {
    // Run for two test positions: -76m and -92m profile
    std::vector<double> test_tx_x = {-64.0, -80.0};  // TX positions for profile -76m and -92m
    std::vector<double> dips = {90.0, 92.0};

    for (double dip : dips) {
        std::cout << "\n========================================\n";
        std::cout << "DIP = " << dip << "°\n";
        std::cout << "========================================\n\n";

        for (double tx_x : test_tx_x) {
            double profile_pos = tx_x - 12.0;

            std::cout << "TX_x = " << std::setw(6) << std::setprecision(1) << std::fixed << tx_x
                      << " m  (profile = " << profile_pos << " m)\n";

            // Simplified parameters
            Vec3 mtx(0, 0, 1);
            double dipoleM = 1847300.1;
            Vec3 rtx(tx_x, 0, 120);
            Vec3 rtxrx(12, 0, 56);
            Vec3 rsp(0, 0, -200);
            double wc = 0.0001546;
            double mu = 1.256637e-6;
            double sigma_ob = 0.0333333333;
            double thick_ob = 4.0;
            double sigma_sp = 0.5;
            double a = 100.0;
            bool applydip = true;
            double strike = 90.0;
            double period = 0.0333333333;

            double H_x, H_y, H_z, H_obx, H_oby, H_obz;
            double convo_x, convo_z;

            H_total_step_1storder(mtx, dipoleM, rtx, rtxrx, rsp, wc, mu,
                                 sigma_ob, thick_ob, sigma_sp, a, applydip,
                                 dip, strike, period,
                                 H_x, H_y, H_z, H_obx, H_oby, H_obz,
                                 convo_x, convo_z);

            double H_x_nT = (mu / 1e-12) * H_x;

            std::cout << "  convo_x = " << std::setw(12) << std::setprecision(2) << convo_x << "\n";
            std::cout << "  convo_z = " << std::setw(12) << std::setprecision(2) << convo_z << "\n";
            std::cout << "  H_x = " << std::setw(8) << std::setprecision(1) << H_x_nT << " nT\n\n";
        }
    }

    std::cout << "\nExpected:\n";
    std::cout << "  dip=90 @ profile -76m: -61 nT\n";
    std::cout << "  dip=92 @ profile -76m: -66 nT\n";

    return 0;
}
