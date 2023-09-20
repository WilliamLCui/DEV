//
// Created by Willi on 2023/8/23.
//

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <future>

cv::Mat generate_white_img(const std::vector<int>& map_, int h, int w) {
    cv::Mat img(h, w, CV_8UC3, cv::Scalar(0, 0, 0)); // Create a black image

    for (int j = 0; j < map_.size(); j += 2) {
        int dst = map_[j + 1];
        int y = dst / w;
        int x = dst % w;
        img.at<cv::Vec3b>(y, x) = cv::Vec3b(255, 255, 255);
    }

    return img;
}

std::pair<int, int> get_x_min_max(int x_min, int x_max) {
    x_min = x_min - 1;
    x_max = x_max + 2;
    return std::make_pair(x_min, x_max);
}

cv::Mat get_weight(const std::vector<int>& map_, std::vector<double>& weights, const std::vector<int>& xs_min_left, const std::vector<int>& xs_max_left,
                   const std::vector<int>& xs_min_right, const std::vector<int>& xs_max_right, int w, double ratio = 1) {
    for (int i = 0; i < map_.size(); i += 2) {
        int dst = map_[i + 1];
        int y = dst / w;
        int x = dst % w;

        std::pair<int, int> x_range_left = get_x_min_max(xs_min_left[y], xs_max_left[y]);
        std::pair<int, int> x_range_right = get_x_min_max(xs_min_right[y], xs_max_right[y]);

        if (x_range_left.first <= x && x <= x_range_left.second) {
            double dist_left = x - x_range_left.first;
            double dist_right = x_range_left.second - x;
            dist_left *= ratio;
            weights[i / 2] = dist_left / (dist_left + dist_right);
        }
        else if (x_range_right.first <= x && x <= x_range_right.second) {
            double dist_left = x - x_range_right.first;
            double dist_right = x_range_right.second - x;
            dist_left *= ratio;
            weights[i / 2] = dist_left / (dist_left + dist_right);
        }
    }
}


void get_x_min_max(int xs_min[], int xs_max[], Mat img_a, Mat img_b, int w, int h) {
    Mat mask_overlap;
    bitwise_and(img_a, img_b, mask_overlap);

    Mat gray;
    cvtColor(mask_overlap, gray, COLOR_BGR2GRAY);

    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(gray, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    int x_min_up = w;
    int x_max_up = 0;
    int x_min_down = w;
    int x_max_down = 0;
    for (size_t i = 0; i < contours[0].size(); i++) {
        Point coord = contours[0][i];
        int x = coord.x;
        int y = coord.y;
        if (y == 0) {
            if (x > x_max_up) {
                x_max_up = x;
            }
            if (x < x_min_up) {
                x_min_up = x;
            }
        } else if (y == h - 1) {
            if (x > x_max_down) {
                x_max_down = x;
            }
            if (x < x_min_down) {
                x_min_down = x;
            }
        }
    }

    xs_min[0] = x_min_up;
    xs_max[0] = x_max_up;
    xs_min[h - 1] = x_min_down;
    xs_max[h - 1] = x_max_down;
}

std::vector<double> get_weight(const std::vector<int>& map_, const std::vector<double>& weights, int* xs_min_left, int* xs_max_left, int* xs_min_right, int* xs_max_right, int w, double ratio = 1) {
    std::vector<double> result = weights;

    for (int i = 0; i < map_.size(); i += 2) {
        int dst = map_[i + 1];
        int y = dst / w;
        int x = dst % w;

        int x_min_left, x_max_left, x_min_right, x_max_right;
        std::get_x_min_max(&x_min_left, &x_max_left, xs_min_left[y], xs_max_left[y]);
        std::get_x_min_max(&x_min_right, &x_max_right, xs_min_right[y], xs_max_right[y]);

        if (x_min_left <= x && x <= x_max_left) {
            int dist_left = x - x_min_left;
            int dist_right = x_max_left - x;
            dist_left *= ratio;
            result[i / 2] = dist_left / (dist_left + dist_right);
        } else if (x_min_right <= x && x <= x_max_right) {
            int dist_left = x - x_min_right;
            int dist_right = x_max_right - x;
            dist_left *= ratio;
            result[i / 2] = dist_right / (dist_left + dist_right);
        }
    }

    return result;
}

std::vector<double> get_weight(const std::vector<int>& map_, std::vector<double>& weights, const std::vector<int>& xs_min_left, const std::vector<int>& xs_max_left, const std::vector<int>& xs_min_right, const std::vector<int>& xs_max_right, int w, double ratio = 1) {
    for (int i = 0; i < map_.size(); i += 2) {
        int dst = map_[i + 1];
        int y = dst / w;
        int x = dst % w;

        int x_min_left, x_max_left;
        std::tie(x_min_left, x_max_left) = get_x_min_max(xs_min_left[y], xs_max_left[y]);

        int x_min_right, x_max_right;
        std::tie(x_min_right, x_max_right) = get_x_min_max(xs_min_right[y], xs_max_right[y]);

        // 融合部分
        if (x_min_left <= x && x <= x_max_left) {
            int dist_left = x - x_min_left;
            int dist_right = x_max_left - x;
            dist_left *= ratio;
            weights[i / 2] = dist_left / (dist_left + dist_right);
        } else if (x_min_right <= x && x <= x_max_right) {
            int dist_left = x - x_min_right;
            int dist_right = x_max_right - x;
            dist_left *= ratio;
            weights[i / 2] = dist_right / (dist_left + dist_right);
        }
    }

    return weights;
}

cv::Mat get_xs_min_max(const cv::Mat& img_a, const cv::Mat& img_b, int w, int h) {
    cv::Mat mask_overlap;
    cv::bitwise_and(img_a, img_b, mask_overlap);

    cv::Mat gray;
    cv::cvtColor(mask_overlap, gray, cv::COLOR_BGR2GRAY);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(gray, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    int x_min_up = w;
    int x_max_up = 0;
    int x_min_down = w;
    int x_max_down = 0;
    for (const auto& coord : contours[0]) {
        int x = coord.x;
        int y = coord.y;
        if (y == 0) {
            if (x > x_max_up) {
                x_max_up = x;
            }
            if (x < x_min_up) {
                x_min_up = x;
            }
        } else if (y == h - 1) {
            if (x > x_max_down) {
                x_max_down = x;
            }
            if (x < x_min_down) {
                x_min_down = x;
            }
        }
    }

    cv::Mat xs_min(h, 1, CV_32SC1);
    cv::Mat xs_max(h, 1, CV_32SC1);
    for (int i = 0; i < h; i++) {
        xs_min.at<int>(i, 0) = static_cast<int>(std::round((x_min_up * (h - i - 1) + x_min_down * i) / (h - 1)));
        xs_max.at<int>(i, 0) = static_cast<int>(std::round((x_max_up * (h - i - 1) + x_max_down * i) / (h - 1)));
    }

    cv::Mat result;
    cv::hconcat(xs_min, xs_max, result);

    return result;
}

std::vector<float> generate_weights(const Info& info, const std::vector<cv::Mat>& map_far_new, float ratio) {
    std::cout << "Generating weights ..." << std::endl;
    std::vector<float> weights(info.imgs_num);

    std::vector<std::future<float>> tasks;
    for (int i = 0; i < info.imgs_num; i++) {
        const cv::Mat& map_p = map_far_new[(i + info.imgs_num - 1) % info.imgs_num];
        const cv::Mat& map_ = map_far_new[i];
        const cv::Mat& map_q = map_far_new[(i + 1) % info.imgs_num];

        tasks.emplace_back(std::async(std::launch::async, get_weights_, map_p, map_, map_q, info.h_panorama_out, info.w_panorama_out, ratio));
    }

    for (auto& task : tasks) {
        task.wait();
    }

    for (int i = 0; i < info.imgs_num; i++) {
        weights[i] = tasks[i].get();
    }

    return weights;
}


std::vector<std::vector<float>> generate_weights_colorful_balance(const Info& info, const std::vector<ImgModel>& imgs_models, const std::vector<std::vector<float>>& weights) {
    std::cout << "Balancing colorfulness ..." << std::endl;
    std::vector<float> light_balance(info.imgs_num);
    for (int i = 0; i < info.imgs_num; i++) {
        const cv::Mat& img = imgs_models[i].img_ori;
        light_balance[i] = cv::mean(img)[0];
    }

    float light_mean = cv::mean(light_balance)[0];

    std::vector<float> light_balance_w(info.imgs_num);
    for (int i = 0; i < info.imgs_num; i++) {
        float ratio = light_mean / light_balance[i];
        ratio = std::min(1.0f, ratio);
        light_balance_w[i] = ratio;
    }

    std::vector<std::vector<float>> weights_n(info.imgs_num);
    for (int i = 0; i < info.imgs_num; i++) {
        const std::vector<float>& weight = weights[i];
        std::vector<float> weight_n(weight.size());
        for (int j = 0; j < weight.size(); j++) {
            float temp = weight[j] * light_balance_w[i];
            weight_n[j] = temp;
        }
        weights_n[i] = weight_n;
    }

    return weights_n;
}

int main() {
    // Example usage of the functions
    std::vector<int> map_ = { /* fill in your LUT data */ };
    int h = 480; // Example height
    int w = 640; // Example width

    cv::Mat white_img = generate_white_img(map_, h, w);
    // Use white_img as needed

    std::vector<int> xs_min_left = { /* fill in your data */ };
    std::vector<int> xs_max_left = { /* fill in your data */ };
    std::vector<int> xs_min_right = { /* fill in your data */ };
    std::vector<int> xs_max_right = { /* fill in your data */ };
    int panorama_width = 1920; // Example panorama width
    std::vector<double> weights(map_.size() / 2, 1.0);

    get_weight(map_, weights, xs_min_left, xs_max_left, xs_min_right, xs_max_right, panorama_width);
    // Use weights as needed

    return 0;
}