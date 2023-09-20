//
// Created by Willi on 2023/8/19.
//
#include <cmath>
#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
using namespace std;

int get_dx(int f, int yaw) {
    // 根据偏航角yaw角度获得画面横向位移距离。
    double radians = M_PI / 180.0 * yaw;
    int dx = static_cast<int>(f * radians);
    return dx;
}

int get_dy(int f, int pitch) {
    // 根据俯仰角pitch角度获得画面纵向位移距离。
    double radians = M_PI / 180.0 * pitch;
    int dy = static_cast<int>(f * std::tan(radians));
    return dy;
}

bool display(cv::Mat img, std::string name) {
    // 保存图片，或者显示测试图。
    if (name != "test") {
        cv::imwrite(name, img);
    } else {
        cv::imshow(name, img);
        cv::waitKey(0);
    }

    return true;
}


template<typename T>

vector<T> load_map(const std::string& path) {
    /*
    读取二进制文件，以特定数据格式读出并返回。
    */
    std::ifstream file(path, std::ios::binary);
    vector<T> map_;

    if (file) {
        file.seekg(0, std::ios::end);
        std::streampos fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        map_.resize(fileSize / sizeof(T));
        file.read(reinterpret_cast<char*>(map_.data()), fileSize);
    }
    else {
        std::cerr << "Error: Unable to open file - " << path << std::endl;
    }

    return map_;
}

std::vector<std::vector<T>> load_maps(const std::string& maps_root = "maps/dstmap_", int nums = 6) {
    /*
    读取已有的LUT文件。
    */
    std::vector<std::vector<T>> maps;
    for (int i = 0; i < nums; i++) {
        std::string map_path = maps_root + std::to_string(i) + ".bin";
        std::vector<T> map_ = load_map<T>(map_path);
        maps.push_back(map_);
    }
    return maps;
}


void save_map(const std::string& root, const std::vector<T>& map_) {
    std::ofstream file(root + ".bin", std::ios::binary);
    file.write(reinterpret_cast<const char*>(map_.data()), map_.size() * sizeof(T));
}


void save_maps(const std::string& root, const std::vector<std::vector<T>>& maps) {
    std::cout << "Saving " << root << " ..." << std::endl;
    for (std::size_t i = 0; i < maps.size(); ++i) {
        const std::vector<T>& map_ = maps[i];
        save_map(root + std::to_string(i), map_);
    }
}


cv::Mat process_generate_panorama_result(const Info& info, const std::vector<ImgModel>& imgs_models, int i,
                                         const std::vector<int>& map_i, const std::vector<float>& weight_i,
                                         cv::Mat& img_panorama_out) {
    for (std::size_t idx = 0; idx < map_i.size(); idx += 2) {
        int src = map_i[idx];
        int dst = map_i[idx + 1];

        int y_ori = src / info.w_img;
        int x_ori = src % info.w_img;

        float weight = weight_i[idx / 2];

        int y_out = dst / info.w_panorama_out;
        int x_out = dst % info.w_panorama_out;
        img_panorama_out.at<float>(y_out, x_out) = imgs_models[i].img_ori.at<float>(y_ori, x_ori) * weight;
    }

    return img_panorama_out;
}


void display(const vector<vector<vector<float>>>& img_panorama_out, const std::string& filename) {
    // Display the panorama image
    // Implementation not shown
}

std::vector<vector<std::vector<float>>> generate_panorama_result(const Info& info, const std::vector<ImageModel>& imgs_models, const std::vector<std::vector<int>>& map_far_new, const std::vector<float>& weights, const std::string& root_result, bool test_flag) {
    std::cout << "Generating panorama img ..." << std::endl;
    std::vector<std::vector<std::vector<float>>> img_panorama_out(info.h_panorama_out, std::vector<std::vector<float>>(info.w_panorama_out, std::vector<float>(3, 0.0f)));

    std::vector<std::future<void>> tasks;

    for (int i = 0; i < info.imgs_num; ++i) {
        const auto& map_i = map_far_new[i];
        const auto& weight_i = weights[i];
        tasks.push_back(std::async(std::launch::async, [&] {
            std::vector<std::vector<std::vector<float>>> img_panorama_out_i(info.h_panorama_out, std::vector<std::vector<float>>(info.w_panorama_out, std::vector<float>(3, 0.0f)));
            for (size_t idx = 0; idx < map_i.size(); idx += 2) {
                int src = map_i[idx];
                int dst = map_i[idx + 1];

                int y_ori = src / info.w_img;
                int x_ori = src % info.w_img;

                float weight = weight_i[idx / 2];

                int y_out = dst / info.w_panorama_out;
                int x_out = dst % info.w_panorama_out;

                img_panorama_out_i[y_out][x_out][0] = imgs_models[i].img_ori[y_ori][x_ori][0] * weight;
                img_panorama_out_i[y_out][x_out][1] = imgs_models[i].img_ori[y_ori][x_ori][1] * weight;
                img_panorama_out_i[y_out][x_out][2] = imgs_models[i].img_ori[y_ori][x_ori][2] * weight;

                img_panorama_out[y_out][x_out][0] += imgs_models[i].img_ori[y_ori][x_ori][0] * weight;
                img_panorama_out[y_out][x_out][1] += imgs_models[i].img_ori[y_ori][x_ori][1] * weight;
                img_panorama_out[y_out][x_out][2] += imgs_models[i].img_ori[y_ori][x_ori][2] * weight;
            }
            // Display img_panorama_out_i if test_flag is true
            // Implementation not shown
        }));
    }

    for (auto& task : tasks) {
        task.wait();
    }

    display(img_panorama_out, root_result + ".jpg");

    return img_panorama_out;
}

cv::Mat generate_panorama_result_without_weights(Info info, std::vector<ImgModel> imgs_models, std::vector<std::vector<int>> map_far_new, std::string root_no_weights) {
    std::cout << "Generating panorama img ..." << std::endl;
    cv::Mat img_panorama_out(info.h_panorama_out, info.w_panorama_out, CV_8UC3, cv::Scalar(0, 0, 0));
    for (int i = 0; i < info.imgs_num; i++) {
        std::vector<int> map_i = map_far_new[i];
        cv::Mat img_panorama_out_i(info.h_panorama_out, info.w_panorama_out, CV_8UC3, cv::Scalar(0, 0, 0));

        for (int idx = 0; idx < map_i.size(); idx += 2) {
            int src = map_i[idx];
            int dst = map_i[idx + 1];

            int y_ori = src / info.w_img;
            int x_ori = src % info.w_img;

            int y_out = dst / info.w_panorama_out;
            int x_out = dst % info.w_panorama_out;

            img_panorama_out_i.at<cv::Vec3b>(y_out, x_out) = imgs_models[i].img_ori.at<cv::Vec3b>(y_ori, x_ori);
            img_panorama_out.at<cv::Vec3b>(y_out, x_out) += imgs_models[i].img_ori.at<cv::Vec3b>(y_ori, x_ori);
        }

        cv::imwrite(root_no_weights + std::to_string(i) + ".jpg", img_panorama_out_i);
    }

    cv::imwrite(root_no_weights + ".jpg", img_panorama_out);

    return img_panorama_out;
}

std::vector<int> process_project_imgs2panorama(Info info, std::vector<ImgModel> imgs_models, std::vector<int> map_cyl, std::string root_proj, bool test_flag, int i) {
    cv::Mat img_ori = imgs_models[i].img_ori;
    std::vector<int> map_i;

    cv::Mat img_panorama;
    if (test_flag)
        img_panorama = imgs_models[i].img_panorama;

    for (int y = 0; y < info.h_panorama; y++) {
        for (int x = 0; x < info.w_panorama; x++) {
            cv::Mat coord = imgs_models[i].map_inv * cv::Matx31f(x, y, 1.0f);
            int x_src = static_cast<int>(coord.at<float>(0));
            int y_src = static_cast<int>(coord.at<float>(1));

            if (y_src >= 0 && y_src < info.h_img && x_src >= 0 && x_src < info.w_img) {
                int src = y_src * info.w_img + x_src;
                src = map_cyl[src];

                if (!src)
                    continue;

                y_src = src / info.w_img;
                x_src = src % info.w_img;

                int dst = y * info.w_panorama + x;
                map_i.push_back(src);
                map_i.push_back(dst);

                if (test_flag)
                    img_panorama.at<cv::Vec3b>(y, x) = img_ori.at<cv::Vec3b>(y_src, x_src);
            }
            else {
                int x_n = x + info.w_panorama;
                coord = imgs_models[i].map_inv * cv::Matx31f(x_n, y, 1.0f);
                x_src = static_cast<int>(coord.at<float>(0));
                y_src = static_cast<int>(coord.at<float>(1));

                if (y_src >= 0 && y_src < info.h_img && x_src >= 0 && x_src < info.w_img) {
                    int src = y_src * info.w_img + x_src;
                    src = map_cyl[src];

                    if (!src)
                        continue;

                    y_src = src / info.w_img;
                    x_src = src % info.w_img;

                    int dst = y * info.w_panorama + x;
                    map_i.push_back(src);
                    map_i.push_back(dst);

                    if (test_flag)
                        img_panorama.at<cv::Vec3b>(y, x) = img_ori.at<cv::Vec3b>(y_src, x_src);
                }
            }
        }
    }

    if (test_flag) {
        imgs_models[i].img_panorama = img_panorama;
        cv::imwrite(root_proj + std::to_string(1000 + i) + ".jpg", img_panorama);
    }

    return map_i;
}