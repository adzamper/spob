#define LIBRARY_BUILD
#include "sphere_overburden.cpp"
#include <iostream>
#include <iomanip>

int main() {
    std::string params = R"({
        "radar": 120.0, "dipole_m": 1847300.1,
        "rtxrx_x": 12.0, "rtxrx_y": 0.0, "rtxrx_z": 56.0,
        "rsp_x": 0.0, "rsp_y": 0.0, "rsp_z": -200.0,
        "a": 100.0, "sigma_sp": 0.5,
        "sigma_ob": 0.0333333333333333, "thick_ob": 4.0,
        "period": 0.0333333333333333,
        "apply_dip": true, "strike": 90.0, "dip": 90.0,
        "xsign_negative": false,
        "mtx_x": 0.0, "mtx_y": 0.0, "mtx_z": 1.0
    })";

    std::string result = calculate_em_response(params);
    
    // Parse and find peaks (simplified)
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "Testing with rtxrx_x=12m, apply_dip=true, dip=90:\n";
    std::cout << "Expected: neg=-61 nT @ -76m, pos=79 nT @ 68m\n\n";
    std::cout << "Run ../test_all_windows with rtxrx_x=12 to see detailed results...\n";
    
    return 0;
}
