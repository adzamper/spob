// Test ALL time windows to find which matches user's expected values
#define LIBRARY_BUILD
#include "sphere_overburden.cpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <sstream>

// Find peaks
void find_peaks(const std::vector<double>& x_vals, const std::vector<double>& y_vals,
                double& neg_peak_val, double& neg_peak_pos,
                double& pos_peak_val, double& pos_peak_pos) {
    neg_peak_val = 1e9;
    pos_peak_val = -1e9;
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

// Parse JSON array (2D for components)
std::vector<std::vector<double>> parse_2d_array(const std::string& json, const std::string& key) {
    std::vector<std::vector<double>> result;
    size_t pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) return result;

    pos = json.find("[", pos);
    if (pos == std::string::npos) return result;

    size_t end_pos = json.find("]", pos);
    int bracket_count = 1;
    size_t search_pos = pos + 1;
    while (bracket_count > 0 && search_pos < json.size()) {
        if (json[search_pos] == '[') bracket_count++;
        else if (json[search_pos] == ']') bracket_count--;
        search_pos++;
    }
    end_pos = search_pos - 1;

    std::string array_content = json.substr(pos + 1, end_pos - pos - 1);

    // Split by ],[ to get each row
    size_t row_start = 0;
    while (row_start < array_content.size()) {
        size_t row_end = array_content.find("],[", row_start);
        if (row_end == std::string::npos) row_end = array_content.size();

        std::string row_str = array_content.substr(row_start, row_end - row_start);
        // Remove leading/trailing brackets
        if (row_str[0] == '[') row_str = row_str.substr(1);
        if (row_str[row_str.size()-1] == ']') row_str = row_str.substr(0, row_str.size()-1);

        std::vector<double> row;
        std::istringstream iss(row_str);
        std::string num;
        while (std::getline(iss, num, ',')) {
            try {
                row.push_back(std::stod(num));
            } catch (...) {}
        }
        if (!row.empty()) result.push_back(row);

        row_start = row_end + 2;
        if (row_start >= array_content.size()) break;
    }

    return result;
}

std::vector<double> parse_1d_array(const std::string& json, const std::string& key) {
    std::vector<double> result;
    size_t pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) return result;
    pos = json.find("[", pos);
    if (pos == std::string::npos) return result;
    size_t end_pos = json.find("]", pos);
    std::string array_content = json.substr(pos + 1, end_pos - pos - 1);
    std::istringstream iss(array_content);
    std::string num;
    while (std::getline(iss, num, ',')) {
        try { result.push_back(std::stod(num)); } catch (...) {}
    }
    return result;
}

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

    std::vector<double> x_values = parse_1d_array(result, "x_values");
    std::vector<std::vector<double>> x_components = parse_2d_array(result, "x_components");

    std::cout << std::fixed << std::setprecision(1);
    std::cout << "PEAKS FOR EACH TIME WINDOW:\n";
    std::cout << "============================\n\n";

    for (size_t tw = 0; tw < x_components.size(); tw++) {
        double neg_val, neg_pos, pos_val, pos_pos;
        find_peaks(x_values, x_components[tw], neg_val, neg_pos, pos_val, pos_pos);

        std::cout << "Window " << (tw+1) << ": ";
        std::cout << "neg=" << std::setw(7) << neg_val << " nT @ " << std::setw(6) << neg_pos << "m, ";
        std::cout << "pos=" << std::setw(7) << pos_val << " nT @ " << std::setw(6) << pos_pos << "m\n";
    }

    std::cout << "\nExpected: neg=-61 nT @ -76m, pos=79 nT @ 68m\n";
    return 0;
}
