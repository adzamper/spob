// Check if user is using TX position vs profile position
#define LIBRARY_BUILD
#include "sphere_overburden.cpp"
#include <iostream>
#include <iomanip>

int main() {
    std::cout << "COORDINATE SYSTEM EXPLANATION:\n";
    std::cout << "==============================\n\n";
    std::cout << "In the code:\n";
    std::cout << "- TX moves along X from -800m to +800m\n";
    std::cout << "- RX is offset from TX by rtxrx = (125, 0, 56)m\n";
    std::cout << "- Profile position = TX_x - rtxrx_x = TX_x - 125m\n\n";

    std::cout << "Example positions:\n";
    std::cout << "TX_x    | Profile_pos | RX_x\n";
    std::cout << "--------|-------------|------\n";
    std::cout << " -800m  |    -925m    | -925m\n";
    std::cout << "  -76m  |    -201m    | -201m\n";
    std::cout << "   49m  |     -76m    |  -76m  <- If user means profile=-76m\n";
    std::cout << "   68m  |     -57m    |  -57m\n";
    std::cout << "  193m  |      68m    |   68m  <- If user means profile=+68m\n";
    std::cout << "  800m  |     675m    |  675m\n\n";

    std::cout << "HYPOTHESIS: User's \"position\" refers to TX_x, not profile position!\n\n";

    std::cout << "If negative peak is at TX_x=-76m:\n";
    std::cout << "  -> Profile position would be -76-125 = -201m\n\n";

    std::cout << "If positive peak is at TX_x=+68m:\n";
    std::cout << "  -> Profile position would be 68-125 = -57m\n\n";

    std::cout << "Let me check what peaks we get in terms of TX_x...\n\n";

    // Run calculation
    std::string params = R"({
        "radar": 120.0, "dipole_m": 1847300.0,
        "rtxrx_x": 125.0, "rtxrx_y": 0.0, "rtxrx_z": 56.0,
        "rsp_x": 0.0, "rsp_y": 0.0, "rsp_z": -200.0,
        "a": 100.0, "sigma_sp": 0.5,
        "sigma_ob": 0.0333333333333333, "thick_ob": 4.0,
        "period": 0.0333333333333333,
        "apply_dip": false, "strike": 90.0, "dip": 90.0,
        "xsign_negative": false,
        "mtx_x": 0.0, "mtx_y": 0.0, "mtx_z": 1.0
    })";

    // We know peaks are at profile positions -141m and +67m
    double profile_neg = -141.0;
    double profile_pos = 67.0;
    double rtxrx_x = 125.0;

    double tx_neg = profile_neg + rtxrx_x;  // TX_x = profile + rtxrx_x
    double tx_pos = profile_pos + rtxrx_x;

    std::cout << std::fixed << std::setprecision(1);
    std::cout << "Our calculated peaks:\n";
    std::cout << "  Negative peak:\n";
    std::cout << "    Profile position: " << profile_neg << "m\n";
    std::cout << "    TX position:      " << tx_neg << "m  <- Is this -76m?\n\n";
    std::cout << "  Positive peak:\n";
    std::cout << "    Profile position: " << profile_pos << "m\n";
    std::cout << "    TX position:      " << tx_pos << "m  <- Is this +68m?\n\n";

    std::cout << "RESULT: TX peaks are at " << tx_neg << "m and " << tx_pos << "m\n";
    std::cout << "USER expects: -76m and +68m\n\n";

    if (std::abs(tx_neg - (-76.0)) < 10 && std::abs(tx_pos - 68.0) < 125) {
        std::cout << "✗ TX position doesn't match either\n";
    }

    std::cout << "\nMaybe the issue is rtxrx_x is wrong? Let me try rtxrx_x=12...\n";

    return 0;
}
