//
// Created by Willi on 2023/8/20.
//

#include "param_settings.cpp"
#include "utils.cpp"

#include <cmath>
#include <opencv2/opencv.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>


class Info {
private:
    double vision;
    int w_img;
    int h_img;
    int center_x;
    int center_y;
    int imgs_num;
    int w_panorama;
    int h_panorama;
    int x_panorama_center_start;
    int y_panorama_center_start;
    double w_panorama_out;
    double h_panorama_out;

public:
    Info(int w, int h, double vision, int imgs_num, int w_panorama, int h_panorama) {
        this->vision = vision;
        this->w_img = w;
        this->h_img = h;
        this->center_x = w / 2;
        this->center_y = h / 2;
        this->imgs_num = imgs_num;
        this->w_panorama = w_panorama;
        this->h_panorama = h_panorama;
        this->x_panorama_center_start = 0;
        this->y_panorama_center_start = h_panorama / 2 - h_img / 2;

        set_w_h_panorama(w_panorama * w_scale, h_panorama * h_scale);
        this->w_panorama_out = w_panorama_out;
        this->h_panorama_out = h_panorama_out;
    }

    double get_f() {
        return w_img / (2 * std::tan(M_PI / 180 * (vision / 2)));
    }

    int get_proj_width() {
        double f = get_f();
        return static_cast<int>(f * M_PI / 180 * vision);
    }

    void set_w_h_panorama(double w, double h) {
        this->w_panorama = static_cast<int>(w);
        this->h_panorama = static_cast<int>(h);
        this->x_panorama_center_start = w_panorama / 2 - w_img / 2;
        this->y_panorama_center_start = h_panorama / 2 - h_img / 2;
    }
};


class Img {
public:
    cv::Mat img_ori;
    double yaw;
    double roll;
    double pitch;
    double scale;
    double dx;
    double dy;
    double offset_x;
    double offset_y;
    cv::Mat m_rotate_scale;
    cv::Mat m_trans;
    cv::Mat map;
    cv::Mat map_inv;
    double center_x_pano;
    double center_y_pano;
    cv::Mat img_panorama;

    Img(std::string img_path, std::map<std::string, double> euler_angles, Info info, double dx_0, double dy_0) {
        img_ori = cv::imread(img_path);

        yaw = euler_angles["yaw"];
        roll = -euler_angles["roll"];
        pitch = euler_angles["pitch"];
        scale = euler_angles["scale"];

        dx = get_dx(info.get_f(), yaw);
        dy = get_dy(info.get_f(), pitch);
        offset_x = std::fmod((dx - dx_0 + info.x_panorama_center_start), info.w_panorama);
        offset_y = -dy + dy_0 + info.y_panorama_center_start;

        m_rotate_scale = cv::getRotationMatrix2D(cv::Point2f(info.center_x, info.center_y), roll, scale);
        m_trans = (cv::Mat_<float>(2, 3) << 1, 0, offset_x,
                0, 1, offset_y);
        map = set_map();
        map_inv = set_map_inv();

        cv::Mat center = (cv::Mat_<float>(1, 3) << info.center_x, info.center_y, 1);
        cv::Mat transformed_center = map * center.t();
        center_x_pano = transformed_center.at<float>(0, 0);
        center_y_pano = transformed_center.at<float>(1, 0);

        img_panorama = cv::Mat::zeros(info.h_panorama, info.w_panorama, CV_8UC3);
    }

    cv::Mat set_map() {
        cv::Mat temp = m_rotate_scale.clone();
        temp.at<float>(0, 2) += offset_x;
        temp.at<float>(1, 2) += offset_y;
        return temp;
    }

    cv::Mat set_map_inv() {
        cv::Mat temp = cv::Mat::zeros(3, 3, CV_32FC1);
        cv::vconcat(map, (cv::Mat_<float>(1, 3) << 0, 0, 1), temp);
        cv::Mat temp_inv = temp.inv();
        return temp_inv;
    }
};