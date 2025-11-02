// Test to reproduce dip=90 vs dip=92 issue
// This will run the full profile calculation and find peaks

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

    // Base parameters matching MATLAB defaults
    std::string base_params = R"({
        "radar": 120,
        "dipole_m": 1847300.1,
        "rtxrx_x": 125,
        "rtxrx_y": 0,
        "rtxrx_z": 56,
        "rsp_x": 0,
        "rsp_y": 0,
        "rsp_z": -200,
        "a": 100,
        "sigma_sp": 0.5,
        "sigma_ob": 0.0333333333,
        "thick_ob": 4,
        "period": 0.0333333333,
        "apply_dip": false,
        "strike": 90,
        "xsign_negative": false,
        "mtx_x": 0,
        "mtx_y": 0,
        "mtx_z": 1
    })";

    std::vector<double> dip_angles = {90.0, 92.0};

    std::cout << "========================================\n";
    std::cout << "TESTING DIP ANGLE VARIATION\n";
    std::cout << "========================================\n\n";

    for (double dip : dip_angles) {
        // Add dip angle to params
        std::string params = base_params;
        size_t insert_pos = params.find_last_of("}");
        params.insert(insert_pos, ", \"dip\": " + std::to_string(dip));

        std::cout << "Running calculation with dip = " << dip << "°\n";
        std::cout << "----------------------------------------\n";

        // Run calculation
        std::string result = calculate_em_response(params);

        // Parse results
        std::vector<double> x_values = parse_json_array(result, "x_values");
        std::vector<double> x_components = parse_json_array(result, "x_components");

        if (x_values.empty() || x_components.empty()) {
            std::cout << "ERROR: Failed to parse results\n\n";
            continue;
        }

        // Find peaks in X component (first time window)
        double neg_peak_val, neg_peak_pos, pos_peak_val, pos_peak_pos;
        find_peaks(x_values, x_components, neg_peak_val, neg_peak_pos,
                   pos_peak_val, pos_peak_pos);

        std::cout << "X Component Response:\n";
        std::cout << "  Negative peak: " << neg_peak_val << " nT at " << neg_peak_pos << " m\n";
        std::cout << "  Positive peak: " << pos_peak_val << " nT at " << pos_peak_pos << " m\n";
        std::cout << "\n";

        // Print some profile values for debugging
        std::cout << "Sample profile values (first 10 points):\n";
        std::cout << "  X pos (m) | Response (nT)\n";
        std::cout << "  ----------|---------------\n";
        for (size_t i = 0; i < std::min(size_t(10), x_values.size()); i++) {
            std::cout << "  " << std::setw(8) << x_values[i] << " | "
                      << std::setw(12) << x_components[i] << "\n";
        }
        std::cout << "\n";
    }

    std::cout << "========================================\n";
    std::cout << "EXPECTED RESULTS (from user):\n";
    std::cout << "========================================\n";
    std::cout << "dip = 90°:\n";
    std::cout << "  Negative peak: -61 nT at -76 m\n";
    std::cout << "  Positive peak: 79 nT at 68 m\n";
    std::cout << "\n";
    std::cout << "dip = 92° (expected):\n";
    std::cout << "  Negative peak: -66 nT at -76 m\n";
    std::cout << "  Positive peak: 74 nT at 74 m\n";
    std::cout << "========================================\n";

    return 0;
}
