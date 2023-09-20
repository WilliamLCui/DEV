//
// Created by Willi on 2023/8/25.
//
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include"infrared_stitch/read_pto.cpp"
#include"infrared_stitch/cylindrical_project.cpp"
#include"infrared_stitch/get_weights.cpp"
#include"infrared_stitch/scale_panorama.cpp"
#include"infrared_stitch/param_settings.cpp"
#include"infrared_stitch/utils.cpp"
#include"infrared_stitch/structures.cpp"
// Import directives


void create_imgs_models(const Info& info, const std::vector<double>& imgs_euler_angles, double dx_0, double dy_0, const std::string& root_imgs_src) {
    std::cout << "Creating img models ..." << std::endl;

    std::vector<Img> imgs_models;

    for (int i = 0; i < info.imgs_num; i++) {
        std::string path_img = root_imgs_src + std::to_string(i) + ".jpg";
        Img img_model(path_img, imgs_euler_angles[i], info, dx_0, dy_0);
        imgs_models.push_back(img_model);
    }

}

void stitch() {
    std::cout << std::string(40, '=') << std::endl;
    std::cout << std::string(8, '*') << " 红外六幅画面拼接表生成 " << std::string(8, '*') << std::endl;
    std::cout << std::string(8, '*') << " 版本：20230706 " << std::string(8, '*') << std::endl;
    std::cout << std::string(40, '=') << std::endl;

    auto start_time = std::chrono::steady_clock::now();

    if (!std::filesystem::is_directory(settings.dir_imgs)) {
        std::filesystem::create_directory(settings.dir_imgs);
        std::cout << "需要在 'images' 文件夹内放入带拼接的图片！" << std::endl;
        return;
    }
    if (!std::filesystem::is_directory(settings.dir_pto)) {
        std::filesystem::create_directory(settings.dir_pto);
        std::cout << "需要在 'pto' 文件夹内放入 'pto.txt' 基本拼接参数文件！" << std::endl;
        std::cout << "若为新车型，'pto.txt' 需要利用PTGui生成！" << std::endl;
        return;
    }

    if (!std::filesystem::is_directory(settings.dir_maps)) {
        std::filesystem::create_directory(settings.dir_maps);
        std::cout << "新建 'map' 文件夹存放拼接参数结果文件！" << std::endl;
    }
    if (!std::filesystem::is_directory(settings.dir_results)) {
        std::filesystem::create_directory(settings.dir_results);
        std::cout << "新建 'results' 文件夹存放拼接测试结果图片！" << std::endl;
    }

    try {
        Info info;
        std::vector<double> imgs_euler_angles;
        read_pto(settings.root_pto, info, imgs_euler_angles);
    } catch (const std::exception& e) {
        std::cout << "pto 文件读取错误！" << std::endl;
        return;
    }

    try {
        std::vector<double> imgs_euler_angles;
        double overlap_ratio;
        bool test_flag;
        read_adjust(settings.root_adjust, imgs_euler_angles, overlap_ratio, test_flag);
    } catch (const std::exception& e) {
        std::cout << "adjust.txt 文件读取错误！" << std::endl;
        return;
    }

    double dx_0 = get_dx(info.get_f(), imgs_euler_angles[0]["yaw"]);
    double dy_0 = get_dy(info.get_f(), imgs_euler_angles[0]["pitch"]);

    try {
        std::vector<Img> imgs_models = create_imgs_models(info, imgs_euler_angles, dx_0, dy_0, settings.root_imgs_src);
    } catch (const std::exception& e) {
        std::cout << "原始图像读取错误！" << std::endl;
        return;
    }

    Map map_cyl = cylindrical_project(info, 0, 0);
    auto run_time = std::chrono::steady_clock::now() - start_time;
    std::cout << "Run time: " << std::chrono::duration_cast<std::chrono::minutes>(run_time).count() << "m "
              << std::chrono::duration_cast<std::chrono::seconds>(run_time).count() % 60 << "s" << std::endl;

    Map map_proj = project_imgs2panorama(info, imgs_models, map_cyl, settings.root_proj, test_flag);
    run_time = std::chrono::steady_clock::now() - start_time;
    std::cout << "Run time: " << std::chrono::duration_cast<std::chrono::minutes>(run_time).count() << "m "
              << std::chrono::duration_cast<std::chrono::seconds>(run_time).count() % 60 << "s" << std::endl;

    Maps maps = scale_maps(info, map_proj);
    run_time = std::chrono::steady_clock::now() - start_time;
    std::cout << "Run time: " << std::chrono::duration_cast<std::chrono::minutes>(run_time).count() << "m "
              << std::chrono::duration_cast<std::chrono::seconds>(run_time).count() % 60 << "s" << std::endl;

    // Further code...
}