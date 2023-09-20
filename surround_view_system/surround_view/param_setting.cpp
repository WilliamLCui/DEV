#include <iostream>
#include <fstream>
#include <regex>

#include <opencv2/opencv.hpp>

int main() {
    // Read adjust file
    std::string adjust_root = "./adjust.txt";
    std::ifstream file(adjust_root);
    std::string line;

    // Regular expressions to extract values
    std::regex car_w_num_regex("car_w_num:(-?\\d+.?\\d*)");
    std::regex car_h_num_car_w_num_ratio_regex("car_h_num/car_w_num:(-?\\d+.?\\d*)");
    std::regex shift_wl_num_regex("shift_wl_num:(-?\\d+.?\\d*)");
    std::regex shift_ht_num_regex("shift_ht_num:(-?\\d+.?\\d*)");
    std::regex total_w_regex("total_w:(-?\\d+.?\\d*)");
    std::regex total_h_regex("total_h:(-?\\d+.?\\d*)");
    std::regex per_size_regex("per_size:(-?\\d+.?\\d*)");
    std::regex w_num_regex("w_num:(-?\\d+.?\\d*)");
    std::regex h_num_regex("h_num:(-?\\d+.?\\d*)");
    std::regex weight_front_back_a_regex("weight_front_back_a:(-?\\d+.?\\d*)");
    std::regex weight_front_back_b_regex("weight_front_back_b:(-?\\d+.?\\d*)");
    std::regex manual_project_regex("manual_project:(-?\\d+.?\\d*)");

    float car_w_num = 0.0;
    float car_h_num_car_w_num_ratio = 0.0;
    float shift_wl_num = 0.0;
    float shift_ht_num = 0.0;
    float total_w = 0.0;
    float total_h = 0.0;
    float per_size = 0.0;
    float w_num = 0.0;
    float h_num = 0.0;
    float weight_front_back_a = 0.0;
    float weight_front_back_b = 0.0;
    int manual_project = 0;

    if (file.is_open()) {
        int line_count = 0;
        while (std::getline(file, line)) {
            switch (line_count) {
                case 0: {  // car_w_num
                    std::smatch match;
                    if (std::regex_search(line, match, car_w_num_regex)) {
                        car_w_num = std::stof(match[1].str());
                    }
                    break;
                }
                case 1: {  // car_h_num/car_w_num
                    std::smatch match;
                    if (std::regex_search(line, match, car_h_num_car_w_num_ratio_regex)) {
                        car_h_num_car_w_num_ratio = std::stof(match[1].str());
                    }
                    break;
                }
                case 4: {  // shift_wl_num
                    std::smatch match;
                    if (std::regex_search(line, match, shift_wl_num_regex)) {
                        shift_wl_num = std::stof(match[1].str());
                    }
                    break;
                }
                case 5: {  // shift_ht_num
                    std::smatch match;
                    if (std::regex_search(line, match, shift_ht_num_regex)) {
                        shift_ht_num = std::stof(match[1].str());
                    }
                    break;
                }
                case 8: {  // total_w
                    std::smatch match;
                    if (std::regex_search(line, match, total_w_regex)) {
                        total_w = std::stof(match[1].str());
                    }
                    break;
                }
                case 9: {  // total_h
                    std::smatch match;
                    if (std::regex_search(line, match, total_h_regex)) {
                        total_h = std::stof(match[1].str());
                    }
                    break;
                }
                case 11: {  // per_size
                    std::smatch match;
                    if (std::regex_search(line, match, per_size_regex)) {
                        per_size = std::stof(match[1].str());
                    }
                    break;
                }
                case 13: {  // w_num
                    std::smatch match;
                    if (std::regex_search(line, match, w_num_regex)) {
                        w_num = std::stof(match[1].str());
                    }
                    break;
                }
                case 14: {  // h_num
                    std::smatch match;
                    if (std::regex_search(line, match, h_num_regex)) {
                        h_num = std::stof(match[1].str());
                    }
                    break;
                }
                case 16: {  // weight_front_back_a
                    std::smatch match;
                    if (std::regex_search(line, match, weight_front_back_a_regex)) {
                        weight_front_back_a = std::stof(match[1].str());
                    }
                    break;
                }
                case 17: {  // weight_front_back_b
                    std::smatch match;
                    if (std::regex_search(line, match, weight_front_back_b_regex)) {
                        weight_front_back_b = std::stof(match[1].str());
                    }
                    break;
                }
                case 20: {  // manual_project
                    std::smatch match;
                    if (std::regex_search(line, match, manual_project_regex)) {
                        manual_project = std::stoi(match[1].str());
                    }
                    break;
                }
            }
            line_count++;
        }
        file.close();
    }

    // Src images
    std::string images_root = "./images/imgs";

    // Original fisheye image information
    std::vector<std::string> camera_names = {"front", "back", "left", "right"};
    int src_width = 1920;
    int src_height = 1080;

    // Perform further operations with the acquired parameters

    return 0;
}
// --------------------------------------------------------------------
// 前后摄像头画面的权重比增加比例
// weight_front_back = 50; // default
// weight_front_back_b = 50;    // 越大拼接羽化范围越小，b越大，拼接缝隙越靠近左右摄像头，即画面以前后摄像头内容为主
// weight_front_back_a = 1;    // 越大拼接羽化范围越小，a越大，拼接缝隙越靠近前后摄像头，即画面以左右摄像头内容为主

// --------------------------------------------------------------------
double undis_ratio = 1;  // 鱼眼校正后的显示窗口大小缩放倍数
double project_ratio = 1;  // 投影画布的缩放倍数

// --------------------------------------------------------------------
// total width/height of the stitched image
// car_w_num = 1;  // 7 //汽车横向占棋盘格数量10
// car_h_num_car_w_num_ratio = 2.357;
double car_h_num = car_w_num * car_h_num_car_w_num_ratio;  // 10 //汽车纵向占棋盘格数量26

int per_size = 50;  // 9  // 25 棋盘格尺寸
// new yaml guangzhou
// w_num = 11.8;  // 20  // 20     //横向棋盘格总数量29 7.7
// h_num = 18.7;  // 22     //纵向棋盘格总数量38 13 h = 12 : 17.42
// w_num = 11.8;  // 20  // 20     //横向棋盘格总数量29 7.7
// h_num = 17;  // 22     //纵向棋盘格总数量38 13 h = 12 : 17.42

int shift_wl_num = 1;  // 选点据左边界棋盘格数量
int shift_ht_num = 1;  // 选点据上边界棋盘格数量
int shift_bw_num = 2;  // 选点纵向间隔的棋盘格数量

int total_w = static_cast<int>(800 * project_ratio);  // 520 + 2 * shift_w   //标定输出图像宽
int total_h = static_cast<int>(1080 * project_ratio);  // 572 + 2 * shift_h  //标定输出图像高


int b_w = static_cast<int>(w_num * per_size);  // 棋盘格区域横向像素长度
int b_h = static_cast<int>(h_num * per_size);  // 棋盘格区域纵向像素长度
int shift_w = (total_w - b_w) / 2;  // (800-520)/2   //棋盘格距离外边界横向像素长度
int shift_h = (total_h - b_h) / 2;  // (1080-572)/2  //棋盘格距离外边界纵向像素长度


int xl = static_cast<int>((w_num - car_w_num) * per_size / 2 + shift_w);  // 各分块的横向宽度
int xr = total_w - xl;  // 横向的剩余宽度
int yt = static_cast<int>((h_num - car_h_num) * per_size / 2 + shift_h);  // 各分块的纵向长度
int yb = total_h - yt;  // 纵向的剩余长度

int offset_t_r = 0;
// 尾透视偏移值
int offset_w_r = 0;
// 头缩进
int offset_t_h = 0;
// 尾缩进
int offset_w_h = 0;
// 左右透视偏移值
int offset_zy_r = 0;
// 左右缩进 default = offset_tw_r
int offset_zy_h = 0;
std::map<std::string, std::vector<std::pair<int, int>>> project_keypoints = {
        {"front", {
                          {shift_w + shift_wl_num * per_size + offset_t_r, shift_h + shift_ht_num * per_size + offset_t_h},
                          {shift_w + (w_num - shift_wl_num) * per_size - offset_t_r,
                                  shift_h + shift_ht_num * per_size + offset_t_h},
                          {shift_w + shift_wl_num * per_size, shift_h + (shift_ht_num + shift_bw_num) * per_size},
                          {shift_w + (w_num - shift_wl_num) * per_size, shift_h + (shift_ht_num + shift_bw_num) * per_size}
                  }},
        {"back", {
                          {shift_w + shift_wl_num * per_size + offset_w_r, shift_h + shift_ht_num * per_size + offset_w_h},
                          {shift_w + (w_num - shift_wl_num) * per_size - offset_w_r,
                                  shift_h + shift_ht_num * per_size + offset_w_h},
                          {shift_w + shift_wl_num * per_size, shift_h + (shift_ht_num + shift_bw_num) * per_size},
                          {shift_w + (w_num - shift_wl_num) * per_size, shift_h + (shift_ht_num + shift_bw_num) * per_size}
                  }},
        {"left", {
                          {shift_h + shift_wl_num * per_size + offset_zy_r, shift_w + shift_ht_num * per_size + offset_zy_h},
                          {shift_h + (h_num - shift_wl_num) * per_size - offset_zy_r,
                                  shift_w + shift_ht_num * per_size + offset_zy_h},
                          {shift_h + shift_wl_num * per_size, shift_w + (shift_ht_num + shift_bw_num) * per_size},
                          {shift_h + (h_num - shift_wl_num) * per_size, shift_w + (shift_ht_num + shift_bw_num) * per_size}
                  }},
        {"right", {
                          {shift_h + shift_wl_num * per_size + offset_zy_r, shift_w + shift_ht_num * per_size + offset_zy_h},
                          {shift_h + (h_num - shift_wl_num) * per_size - offset_zy_r,
                                  shift_w + shift_ht_num * per_size + offset_zy_h},
                          {shift_h + shift_wl_num * per_size, shift_w + (shift_ht_num + shift_bw_num) * per_size},
                          {shift_h + (h_num - shift_wl_num) * per_size, shift_w + (shift_ht_num + shift_bw_num) * per_size}
                  }}
};