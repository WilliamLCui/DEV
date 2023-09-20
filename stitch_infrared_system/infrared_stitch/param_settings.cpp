//
// Created by Willi on 2023/8/19.
//
#include <iostream>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

// 红外
// 图像原始路径
std::string dir_imgs = "images";
std::string root_imgs_src = fs::current_path().append(dir_imgs).append("output2_").string();

// pto
std::string dir_pto = "pto";
std::string root_pto = fs::current_path().append(dir_pto).append("pto.txt").string();
std::string root_adjust = fs::current_path().append(dir_pto).append("adjust.txt").string();

std::string dir_maps = "maps";
// cylmap root
std::string root_cylmap = fs::current_path().append(dir_maps).append("cylmap.txt").string();
// dstmap root
std::string root_dstmap = fs::current_path().append(dir_maps).append("dstmap_").string();
// maskmap root
std::string root_maskmap = fs::current_path().append(dir_maps).append("maskmap_").string();

// results root
std::string dir_results = "results";
std::string root_cyl = fs::current_path().append(dir_results).append("cyl_").string();
std::string root_proj = fs::current_path().append(dir_results).append("proj_").string();
std::string root_scale = fs::current_path().append(dir_results).append("scale_").string();
std::string root_no_weights = fs::current_path().append(dir_results).append("no_weights_").string();
std::string root_result = fs::current_path().append(dir_results).append("result_").string();

// 设定输出图像的画布大小
int h_panorama_out = 513;
int w_panorama_out = 3841;

// 融合区的宽度收窄率，默认为1，收缩率越大，融合区越窄。
int overlap_ratio = 1;

// # 设定原始图像规格下拼接图的规格缩放
int w_scale = 1;
int h_scale = 1;
