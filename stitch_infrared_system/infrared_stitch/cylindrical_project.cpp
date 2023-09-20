//
// Created by Willi on 2023/8/24

#include <iostream>
#include <vector>
#include <math.h>

struct Info {
    int center_x;
    int center_y;
    int h_img;
    int w_img;

    double get_f() {
        // Implement the logic to retrieve the value of f
    }

    double get_proj_width() {
        // Implement the logic to retrieve the value of proj_width
    }
};

struct ImgModel {
    // Define the ImgModel structure according to your requirements
};

std::pair<int, int> cylindrical_projection_point(int x, int y, const Info& info, int reverse_flag = 0) {
    int center_x = info.center_x;
    int center_y = info.center_y;
    double f = info.get_f();
    double proj_width = info.get_proj_width();

    // Calculate the horizontal angle theta between the camera and the pixel position
    double theta = atan((x - center_x) / f);

    int point_x = int(proj_width / 2 + f * theta);
    int point_y;
    if (!reverse_flag) {
        // Perform cylindrical projection
        point_y = int(f * (y - center_y) / sqrt((x - center_x) * (x - center_x) + f * f) + center_y);
    } else {
        // Perform inverse cylindrical projection (not recommended)
        point_y = int((y - center_y) * sqrt((x - center_x) * (x - center_x) + f * f) / f + center_y);
    }

    return {point_x, point_y};
}

std::vector<std::vector<int>> thread_cylindrical(const Info& info, const std::vector<std::vector<int>>& map_cyl, int idx, int reverse_flag = 0, int no_cylind_flag = 0) {
    std::vector<std::vector<int>> map_i(info.h_img * info.w_img);
    for (int y = 0; y < info.h_img; y++) {
        for (int x = 0; x < info.w_img; x++) {
            int src = y * info.w_img + x;
            int dst;
            if (no_cylind_flag) {
                dst = src;
            } else {
                auto [x_n, y_n] = cylindrical_projection_point(x, y, info, reverse_flag);
                dst = y_n * info.w_img + x_n;
            }
            map_i[dst] = src;
        }
    }

    return map_i;
}

std::vector<int> cylindrical_project(Info info, int reverse_flag = 0, int no_cylind_flag = 0) {
    std::cout << "Cylindrical projecting ..." << std::endl;

    std::vector<int> map_cyl(info.h_img * info.w_img, 0);
    for (int y = 0; y < info.h_img; y++) {
        for (int x = 0; x < info.w_img; x++) {
            int src = y * info.w_img + x;
            int dst;
            if (no_cylind_flag) {
                dst = src;
            } else {
                int x_n, y_n;
                cylindrical_projection_point(x, y, info, reverse_flag, x_n, y_n);
                dst = y_n * info.w_img + x_n;
            }
            map_cyl[dst] = src;
        }
    }

    return map_cyl;
}

void generate_cylindrical_imgs(Info info, std::vector<ImageModel> imgs_models, std::vector<int> map_cyl, std::string root_cyl) {
    std::cout << "Generating cylindrical img ..." << std::endl;
    std::vector<int> test(info.h_img * info.w_img * 3, 0);
    for (int i = 0; i < info.imgs_num; i++) {
        for (int y = 0; y < info.h_img; y++) {
            for (int x = 0; x < info.w_img; x++) {
                int dst = y * info.w_img + x;
                int src = map_cyl[dst];
                if (!src) {
                    src = 0;
                }
                int y_src = src / info.w_img;
                int x_src = src % info.w_img;
                test[3 * (y * info.w_img + x)] = imgs_models[i].img_ori[3 * (y_src * info.w_img + x_src)];
                test[3 * (y * info.w_img + x) + 1] = imgs_models[i].img_ori[3 * (y_src * info.w_img + x_src) + 1];
                test[3 * (y * info.w_img + x) + 2] = imgs_models[i].img_ori[3 * (y_src * info.w_img + x_src) + 2];
                display(test, root_cyl + std::to_string(1000 + i) + ".jpg");
            }
        }
    }
}