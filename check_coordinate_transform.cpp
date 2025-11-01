// Check coordinate transformation consistency
#include <iostream>
#include <iomanip>

int main() {
    std::cout << std::fixed << std::setprecision(6);
    
    // MATLAB parameters
    double rtxrx_x = 12.0;
    double delta_x = 1600.0 / 100.0;  // interval-1 = 100
    
    std::cout << "MATLAB Profile Calculation:" << std::endl;
    std::cout << "============================" << std::endl;
    std::cout << "rtxrx(1) = " << rtxrx_x << std::endl;
    std::cout << "delta_x = " << delta_x << std::endl << std::endl;
    
    std::cout << "Profile position calculations for key positions:" << std::endl;
    std::cout << "tx_x value | profile = tx_x - rtxrx(1)" << std::endl;
    std::cout << "-----------------------------------------------" << std::endl;
    
    double tx_positions[] = {-800, -688, 0, 112, 800};
    for (double tx_x : tx_positions) {
        double profile = tx_x - rtxrx_x;
        std::cout << std::setw(10) << tx_x << " | " << std::setw(10) << profile << std::endl;
    }
    
    std::cout << std::endl << "C++ Profile Calculation:" << std::endl;
    std::cout << "========================" << std::endl;
    std::cout << "profile_length = 800" << std::endl;
    std::cout << "interval = 101" << std::endl;
    std::cout << "delta_x = 2 * 800 / (101-1) = " << (2 * 800.0 / 100.0) << std::endl << std::endl;
    
    double profile_length = 800.0;
    int interval = 101;
    double cpp_delta_x = (2.0 * profile_length) / (interval - 1);
    
    std::cout << "tx_x value | profile = tx_x - rtxrx_x" << std::endl;
    std::cout << "-----------------------------------------------" << std::endl;
    
    for (int i : {0, 25, 50, 75, 100}) {
        double x = -profile_length + i * cpp_delta_x;
        double profile = x - rtxrx_x;
        std::cout << std::setw(10) << x << " | " << std::setw(10) << profile 
                  << "  (i=" << i << ")" << std::endl;
    }
    
    return 0;
}
