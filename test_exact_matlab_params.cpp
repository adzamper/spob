// Test with exact MATLAB parameters from debug log
#include <cmath>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <functional>
#include <limits>
#include <iostream>
#include <iomanip>

// Forward declare the function from sphere_overburden
extern std::string calculate_em_response(const std::string& params_json);

struct PeakInfo {
    double position;
    double value;
    int index;
};

PeakInfo find_peak(const std::vector<double>& positions, const std::vector<double>& values) {
    int max_idx = 0;
    double max_abs = std::abs(values[0]);

    for (size_t i = 1; i < values.size(); i++) {
        if (std::abs(values[i]) > max_abs) {
            max_abs = std::abs(values[i]);
            max_idx = i;
        }
    }

    return {positions[max_idx], values[max_idx], max_idx};
}

std::vector<std::vector<double>> parse_2d_array(const std::string& json, const std::string& key) {
    std::vector<std::vector<double>> result;

    size_t key_pos = json.find("\"" + key + "\"");
    if (key_pos == std::string::npos) return result;

    size_t array_start = json.find("[[", key_pos);
    if (array_start == std::string::npos) return result;

    size_t array_end = json.find("]]", array_start);
    if (array_end == std::string::npos) return result;

    std::string array_str = json.substr(array_start + 1, array_end - array_start);

    // Parse each row
    size_t pos = 0;
    while (pos < array_str.length()) {
        size_t row_start = array_str.find("[", pos);
        if (row_start == std::string::npos) break;

        size_t row_end = array_str.find("]", row_start);
        if (row_end == std::string::npos) break;

        std::string row_str = array_str.substr(row_start + 1, row_end - row_start - 1);

        // Parse values in this row
        std::vector<double> row;
        std::istringstream iss(row_str);
        std::string val;
        while (std::getline(iss, val, ',')) {
            try {
                row.push_back(std::stod(val));
            } catch (...) {}
        }

        if (!row.empty()) {
            result.push_back(row);
        }

        pos = row_end + 1;
    }

    return result;
}

std::vector<double> parse_1d_array(const std::string& json, const std::string& key) {
    std::vector<double> result;

    size_t key_pos = json.find("\"" + key + "\"");
    if (key_pos == std::string::npos) return result;

    size_t array_start = json.find("[", key_pos);
    if (array_start == std::string::npos) return result;

    size_t array_end = json.find("]", array_start);
    if (array_end == std::string::npos) return result;

    std::string array_str = json.substr(array_start + 1, array_end - array_start - 1);

    std::istringstream iss(array_str);
    std::string val;
    while (std::getline(iss, val, ',')) {
        try {
            result.push_back(std::stod(val));
        } catch (...) {}
    }

    return result;
}

int main() {
    std::cout << std::fixed << std::setprecision(6);

    // Test with exact MATLAB parameters from debug log
    std::ostringstream params;
    params << "{";
    params << "\"radar\": 120.0,";
    params << "\"dipole_m\": 1847300.0,";
    params << "\"rtxrx\": {\"x\": 12.0, \"y\": 0.0, \"z\": 56.0},";
    params << "\"rsp\": {\"x\": 0.0, \"y\": 0.0, \"z\": -200.0},";
    params << "\"a\": 100.0,";
    params << "\"sigma_sp\": 0.50,";
    params << "\"sigma_ob\": 0.003333,";
    params << "\"thick_ob\": 4.0,";
    params << "\"period\": 0.033333,";
    params << "\"apply_dip\": true,";
    params << "\"dip\": 85.0,";
    params << "\"strike\": 90.0";
    params << "}";

    std::string params_json = params.str();

    std::cout << "Testing with MATLAB parameters:" << std::endl;
    std::cout << "================================" << std::endl;
    std::cout << "  radar = 120.0 m" << std::endl;
    std::cout << "  dipoleM = 1.847300e+06 A*m^2" << std::endl;
    std::cout << "  rtxrx = [12.0, 0.0, 56.0] m" << std::endl;
    std::cout << "  rsp = [0.0, 0.0, -200.0] m" << std::endl;
    std::cout << "  a = 100.0 m" << std::endl;
    std::cout << "  sigma_sp = 0.50 S/m" << std::endl;
    std::cout << "  sigma_ob = 0.003333 S/m" << std::endl;
    std::cout << "  thick_ob = 4.0 m" << std::endl;
    std::cout << "  applydip = 1" << std::endl;
    std::cout << "  dip = 85.0 deg" << std::endl;
    std::cout << "  strike = 90.0 deg" << std::endl;
    std::cout << "  T (period) = 0.033333 s" << std::endl;
    std::cout << std::endl;

    std::string result_json = calculate_em_response(params_json);

    // Parse results
    std::vector<double> x_values = parse_1d_array(result_json, "x_values");
    std::vector<std::vector<double>> x_components = parse_2d_array(result_json, "x_components");
    std::vector<std::vector<double>> z_components = parse_2d_array(result_json, "z_components");

    if (x_values.empty() || x_components.empty() || z_components.empty()) {
        std::cout << "Error parsing results!" << std::endl;
        return 1;
    }

    std::cout << "Results for dip = 85.0 deg:" << std::endl;
    std::cout << "============================" << std::endl << std::endl;

    // Show first time window (154.6 us)
    std::cout << "TIME WINDOW 1: t = 1.546000e-04 s (154.60 us)" << std::endl;
    std::cout << "----------------------------------------------" << std::endl;

    PeakInfo x_peak = find_peak(x_values, x_components[0]);
    PeakInfo z_peak = find_peak(x_values, z_components[0]);

    std::cout << "Peak locations:" << std::endl;
    std::cout << "  X-component peak: " << x_peak.position << " m (value = " << x_peak.value << " nT)" << std::endl;
    std::cout << "  Z-component peak: " << z_peak.position << " m (value = " << z_peak.value << " nT)" << std::endl;
    std::cout << std::endl;

    std::cout << "MATLAB values (from debug log):" << std::endl;
    std::cout << "  X-component peak: -60.0 m (value = -83.53 nT)" << std::endl;
    std::cout << "  Z-component peak: -124.0 m (value = 174.83 nT)" << std::endl;
    std::cout << std::endl;

    // Now test variation with different dip angles
    std::cout << std::endl << "Testing dip angle variation:" << std::endl;
    std::cout << "============================" << std::endl << std::endl;

    std::vector<double> test_dips = {0, 30, 60, 85, 90, 95, 120, 150, 180};

    std::cout << std::setw(8) << "Dip" << " | "
              << std::setw(12) << "X-peak val" << " | "
              << std::setw(12) << "Z-peak val" << " | "
              << std::setw(12) << "Ratio |Z/X|" << std::endl;
    std::cout << std::string(60, '-') << std::endl;

    for (double dip : test_dips) {
        std::ostringstream test_params;
        test_params << "{";
        test_params << "\"radar\": 120.0,";
        test_params << "\"dipole_m\": 1847300.0,";
        test_params << "\"rtxrx\": {\"x\": 12.0, \"y\": 0.0, \"z\": 56.0},";
        test_params << "\"rsp\": {\"x\": 0.0, \"y\": 0.0, \"z\": -200.0},";
        test_params << "\"a\": 100.0,";
        test_params << "\"sigma_sp\": 0.50,";
        test_params << "\"sigma_ob\": 0.003333,";
        test_params << "\"thick_ob\": 4.0,";
        test_params << "\"period\": 0.033333,";
        test_params << "\"apply_dip\": true,";
        test_params << "\"dip\": " << dip << ",";
        test_params << "\"strike\": 90.0";
        test_params << "}";

        std::string test_result = calculate_em_response(test_params.str());

        std::vector<std::vector<double>> x_comp = parse_2d_array(test_result, "x_components");
        std::vector<std::vector<double>> z_comp = parse_2d_array(test_result, "z_components");

        if (!x_comp.empty() && !z_comp.empty()) {
            PeakInfo xp = find_peak(x_values, x_comp[0]);
            PeakInfo zp = find_peak(x_values, z_comp[0]);

            double ratio = (std::abs(xp.value) > 1e-10) ? std::abs(zp.value) / std::abs(xp.value) : 0.0;

            std::cout << std::setw(8) << dip << " | "
                      << std::setw(12) << xp.value << " | "
                      << std::setw(12) << zp.value << " | "
                      << std::setw(12) << ratio << std::endl;
        }
    }

    std::cout << std::endl;
    std::cout << "Expected: Ratio should change gradually as dip varies" << std::endl;

    return 0;
}
