//
// Created by Willi on 2023/8/23.
//
#include<iostream>
#include<regex>
#include<fstream>
#include"structures.cpp"
using namespace std;

Info read_pto_info(const std::vector<std::string>& lines) {
    int imgs_num = 0;
    std::regex patt_i("i .*");
    for (const std::string& line : lines) {
        std::smatch match;
        if (std::regex_match(line, match, patt_i)) {
            imgs_num++;
        }
    }
    std::cout << "imgs_num: " << imgs_num << std::endl;

    std::regex patt_pano("p w(\\d+) h(\\d+)");
    std::smatch re_pano_match;
    std::regex_match(lines[2], re_pano_match, patt_pano);
    int w_panorama = std::stoi(re_pano_match[1]);
    int h_panorama = std::stoi(re_pano_match[2]);

    std::regex patt_img_info("i .*w(\\d+) h(\\d+) .* v(\\d+\\.\\d*)");
    std::smatch re_info_match;
    std::regex_match(lines[3], re_info_match, patt_img_info);
    if (re_info_match.empty()) {
        std::regex_match(lines[4], re_info_match, patt_img_info);
    }
    int w = std::stoi(re_info_match[1]);
    int h = std::stoi(re_info_match[2]);
    double vision = std::stod(re_info_match[3]);

    Info info(w, h, vision, imgs_num, w_panorama, h_panorama);

    return info;
}

std::vector<std::unordered_map<std::string, double>> read_pto_euler_angle(const std::vector<std::string>& lines) {
    std::vector<std::unordered_map<std::string, double>> angles;
    std::regex patt_angles("i .* y(-?\\d+\\.?\\d*) r(-?\\d+\\.?\\d*) p(-?\\d+\\.?\\d*)");
    for (const std::string& line : lines) {
        std::smatch res;
        if (std::regex_match(line, res, patt_angles)) {
            double yaw = std::stod(res[1]);
            double roll = std::stod(res[2]);
            double pitch = std::stod(res[3]);
            std::unordered_map<std::string, double> temp = {{"yaw", yaw}, {"roll", roll}, {"pitch", pitch}, {"scale", 1.0}};
            angles.push_back(temp);
        }
    }
    return angles;
}

std::tuple<Info, std::vector<std::unordered_map<std::string, double>>> read_pto(const std::string& path) {
    std::ifstream file(path);
    std::vector<std::string> lines;
    std::string line;

    while (std::getline(file, line)) {
        lines.push_back(line);
    }

    Info info = read_pto_info(lines);
    std::vector<std::unordered_map<std::string, double>> imgs_euler_angles = read_pto_euler_angle(lines);

    return std::make_tuple(info, imgs_euler_angles);
}

std::tuple<std::vector<std::unordered_map<std::string, double>>, double, int> read_adjust(const std::string& path, std::vector<std::unordered_map<std::string, double>>& imgs_euler_angles) {
    std::ifstream file(path);
    std::vector<std::string> lines;
    std::string line;

    while (std::getline(file, line)) {
        lines.push_back(line);
    }

    double overlap_ratio = 1.0;
    std::smatch res;
    std::regex overlap_regex("overlap ratio:(-?\\d+\\.?\\d*)");
    if (std::regex_search(lines[0], res, overlap_regex)) {
        overlap_ratio = std::stod(res[1]);
    }

    int test_flag = 0;
    std::regex test_regex("test:(-?\\d+\\.?\\d*)");
    if (std::regex_search(lines.back(), res, test_regex)) {
        test_flag = std::stoi(res[1]);
    }

    for (size_t i = 1; i < imgs_euler_angles.size(); i++) {
        std::regex x_regex("X:(-?\\d+\\.?\\d*)");
        if (std::regex_search(lines[i * 5 + 1], res, x_regex)) {
            double yaw = std::stod(res[1]);
            imgs_euler_angles[i - 1]["yaw"] += yaw;
        }
        std::regex y_regex("Y:(-?\\d+\\.?\\d*)");
        if (std::regex_search(lines[i + 2], res, y_regex)) {
            double pitch = std::stod(res[1]);
            imgs_euler_angles[i - 1]["pitch"] += pitch;
        }
        std::regex r_regex("R:(-?\\d+\\.?\\d*)");
        if (std::regex_search(lines[i + 3], res, r_regex)) {
            double roll = std::stod(res[1]);
            imgs_euler_angles[i - 1]["roll"] += roll;
        }
        std::regex s_regex("S:(-?\\d+\\.?\\d*)");
        if (std::regex_search(lines[i + 4], res, s_regex)) {
            double scale = std::stod(res[1]);
            imgs_euler_angles[i - 1]["scale"] = scale;
        }
    }

    return std::make_tuple(imgs_euler_angles, overlap_ratio, test_flag);
}