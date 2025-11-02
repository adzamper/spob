// Test with EXACT MATLAB default parameters
#define LIBRARY_BUILD
#include "sphere_overburden.cpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <limits>

// Function to find peaks in a profile
void find_peaks(const std::vector<double>& x_vals, const std::vector<double>& y_vals,
                double& neg_peak_val, double& neg_peak_pos,
                double& pos_peak_val, double& pos_peak_pos) {
    neg_peak_val = std::numeric_limits<double>::max();
    pos_peak_val = std::numeric_limits<double>::lowest();
    neg_peak_pos = 0;
    pos_peak_pos = 0;

    for (size_t i = 0; i < y_vals.size(); i++) {
        if (y_vals[i] < neg_peak_val) {
            neg_peak_val = y_vals[i];
            neg_peak_pos = x_vals[i];
        }
        if (y_vals[i] > pos_peak_val) {
            pos_peak_val = y_vals[i];
            pos_peak_pos = x_vals[i];
        }
    }
}

// Parse JSON array
std::vector<double> parse_json_array(const std::string& json, const std::string& key) {
    std::vector<double> result;
    size_t pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) return result;

    pos = json.find("[", pos);
    if (pos == std::string::npos) return result;

    size_t end_pos = json.find("]", pos);
    if (end_pos == std::string::npos) return result;

    std::string array_content = json.substr(pos + 1, end_pos - pos - 1);

    // Parse numbers
    size_t start = 0;
    while (start < array_content.size()) {
        size_t comma = array_content.find(",", start);
        if (comma == std::string::npos) comma = array_content.size();

        std::string num_str = array_content.substr(start, comma - start);
        try {
            result.push_back(std::stod(num_str));
        } catch (...) {}

        start = comma + 1;
    }

    return result;
}

int main() {
    std::cout << std::fixed << std::setprecision(1);

    // EXACT MATLAB defaults from overburden_1storder_nowave.m
    // radar=120.
    // dipoleM=1.847300*1e6
    // rtxrx(1)=125., rtxrx(2)=0., rtxrx(3)=56.
    // rsp(1)=0., rsp(2)=0., rsp(3)=-200.
    // a=100.0
    // sigma_sp=0.5
    // mtx(1)=0., mtx(2)=0., mtx(3)=1.
    // sigma_ob=1/30
    // thick_ob=4
    // bfreq=30 => T=1/30
    // applydip=0 (but we'll test with 1)
    // strike=90.
    // dip=85.

    std::string params_dip90 = R"({
        "radar": 120.0,
        "dipole_m": 1847300.0,
        "rtxrx_x": 125.0,
        "rtxrx_y": 0.0,
        "rtxrx_z": 56.0,
        "rsp_x": 0.0,
        "rsp_y": 0.0,
        "rsp_z": -200.0,
        "a": 100.0,
        "sigma_sp": 0.5,
        "sigma_ob": 0.0333333333333333,
        "thick_ob": 4.0,
        "period": 0.0333333333333333,
        "apply_dip": false,
        "strike": 90.0,
        "dip": 90.0,
        "xsign_negative": false,
        "mtx_x": 0.0,
        "mtx_y": 0.0,
        "mtx_z": 1.0
    })";

    std::cout << "Testing with EXACT MATLAB defaults (dip=90):\n";
    std::cout << "==============================================\n\n";

    std::string result = calculate_em_response(params_dip90);

    std::vector<double> x_values = parse_json_array(result, "x_values");
    std::vector<double> x_components = parse_json_array(result, "x_components");

    if (!x_values.empty() && !x_components.empty()) {
        double neg_peak_val, neg_peak_pos, pos_peak_val, pos_peak_pos;
        find_peaks(x_values, x_components, neg_peak_val, neg_peak_pos,
                   pos_peak_val, pos_peak_pos);

        std::cout << "Results:\n";
        std::cout << "  Negative peak: " << neg_peak_val << " nT at " << neg_peak_pos << " m\n";
        std::cout << "  Positive peak: " << pos_peak_val << " nT at " << pos_peak_pos << " m\n\n";

        std::cout << "Expected (from user):\n";
        std::cout << "  Negative peak: -61 nT at -76 m\n";
        std::cout << "  Positive peak: 79 nT at 68 m\n\n";

        // Print MORE profile values to see full pattern
        std::cout << "Extended profile values:\n";
        for (size_t i = 0; i < x_values.size(); i++) {
            if (i % 5 == 0) {  // Print every 5th point
                std::cout << "  x=" << std::setw(7) << x_values[i]
                          << " m: " << std::setw(9) << x_components[i] << " nT\n";
            }
        }
    }

    return 0;
}
