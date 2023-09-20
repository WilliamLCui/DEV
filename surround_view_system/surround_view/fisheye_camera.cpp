#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/calib3d.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

#include "param_settings.h" // 导入参数设置文件

class FisheyeCameraModel {
public:
    FisheyeCameraModel(const std::string& camera_param_file, const std::string& camera_name) {
        if (!std::ifstream(camera_param_file)) {
            throw std::runtime_error("Cannot find camera param file");
        }

        if (std::find(settings::camera_names.begin(), settings::camera_names.end(), camera_name) == settings::camera_names.end()) {
            throw std::invalid_argument("Unknown camera name: " + camera_name);
        }

        camera_file = camera_param_file;
        this->camera_name = camera_name;
        scale_xy = std::make_pair(1.0, 1.0);
        shift_xy = std::make_pair(0, 0);
        undistort_maps = cv::Mat();
        project_matrix = cv::Mat();
        project_shape = settings::project_shapes[camera_name];
        dst_resolution = cv::Size(settings::total_w, settings::total_h);
        dst_resolution_s = cv::Size(settings::total_h, settings::total_w);
        load_camera_params();
    }

    void load_camera_params() {
        cv::FileStorage fs(camera_file, cv::FileStorage::READ);
        fs["camera_matrix"] >> camera_matrix;
        fs["dist_coeffs"] >> dist_coeffs;
        fs["resolution"] >> resolution;

        cv::Mat scale_xy_mat, shift_xy_mat, project_matrix_mat;
        fs["scale_xy"] >> scale_xy_mat;
        fs["shift_xy"] >> shift_xy_mat;
        fs["project_matrix"] >> project_matrix_mat;

        if (!scale_xy_mat.empty()) {
            scale_xy = std::make_pair(scale_xy_mat.at<double>(0, 0), scale_xy_mat.at<double>(1, 1));
        }

        if (!shift_xy_mat.empty()) {
            shift_xy = std::make_pair(shift_xy_mat.at<double>(0, 0), shift_xy_mat.at<double>(1, 1));
        }

        if (!project_matrix_mat.empty()) {
            project_matrix = project_matrix_mat;
        }

        fs.release();
        update_undistort_maps();
    }

    void update_undistort_maps() {
        cv::Mat new_camera_matrix = camera_matrix.clone();
        new_camera_matrix.at<double>(0, 0) *= scale_xy.first;
        new_camera_matrix.at<double>(1, 1) *= scale_xy.second;
        new_camera_matrix.at<double>(0, 2) += shift_xy.first;
        new_camera_matrix.at<double>(1, 2) += shift_xy.second;

        cv::fisheye::initUndistortRectifyMap(
                camera_matrix, dist_coeffs, cv::Mat(),
                new_camera_matrix, dst_resolution, CV_16SC2, undistort_maps);
    }

private:
    std::string camera_file;
    std::string camera_name;
    cv::Mat camera_matrix;
    cv::Mat dist_coeffs;
    cv::Mat undistort_maps;
    cv::Mat project_matrix;
    std::pair<double, double> scale_xy;
    std::pair<int, int> shift_xy;
    cv::Size project_shape;
    cv::Size dst_resolution;
    cv::Size dst_resolution_s;
};

cv::Mat undistort(cv::Mat image, cv::Mat undistort_maps[2]) {
    cv::Mat result;
    cv::remap(image, result, undistort_maps[0], undistort_maps[1], cv::INTER_LINEAR, cv::BORDER_CONSTANT);
    return result;
}

cv::Mat project(cv::Mat image, cv::Mat project_matrix, cv::Size project_shape) {
    cv::Mat result;
    cv::warpPerspective(image, result, project_matrix, project_shape);
    return result;
}

cv::Mat flip(cv::Mat image, std::string camera_name) {
    if(camera_name == "front") {
        return image.clone();
    }
    else if(camera_name == "back") {
        cv::Mat flipped_image;
        cv::flip(image, flipped_image, -1);
        return flipped_image;
    }
    else if(camera_name == "left") {
        cv::Mat transposed_image;
        cv::transpose(image, transposed_image);
        cv::flip(transposed_image, transposed_image, 0);
        return transposed_image;
    }
    else {
        cv::Mat transposed_image;
        cv::transpose(image, transposed_image);
        cv::flip(transposed_image, transposed_image, 1);
        return transposed_image;
    }
}

void save_data(std::string camera_file, cv::Mat camera_matrix, cv::Mat dist_coeffs, cv::Size resolution, cv::Mat project_matrix,
               float scale_xy, float shift_xy) {
    cv::FileStorage fs(camera_file, cv::FileStorage::WRITE);
    fs << "camera_matrix" << camera_matrix;
    fs << "dist_coeffs" << dist_coeffs;
    fs << "resolution" << resolution;
    fs << "project_matrix" << project_matrix;
    fs << "scale_xy" << scale_xy;
    fs << "shift_xy" << shift_xy;
    fs.release();
}

void save_maps(std::string camera_name, cv::Mat undistort_maps[2], cv::Mat project_matrix, cv::Size dst_resolution,
               cv::Size dst_resolution_s) {
    cv::Mat map1, map2;
    if(camera_name == "front" || camera_name == "back") {
        cv::warpPerspective(undistort_maps[0], map1, project_matrix, dst_resolution);
        cv::warpPerspective(undistort_maps[1], map2, project_matrix, dst_resolution);
    }
    else {
        cv::warpPerspective(undistort_maps[0], map1, project_matrix, dst_resolution_s);
        cv::warpPerspective(undistort_maps[1], map2, project_matrix, dst_resolution_s);
    }

    // Save map1 to xmap file
    FILE* file = fopen(("maps/xmap_" + camera_name + ".txt").c_str(), "wb");
    if(file != NULL) {
        fwrite(map1.data, sizeof(float), map1.rows * map1.cols, file);
        fclose(file);
    }

    // Save map2 to ymap file
    file = fopen(("maps/ymap_" + camera_name + ".txt").c_str(), "wb");
    if(file != NULL) {
        fwrite(map2.data, sizeof(float), map2.rows * map2.cols, file);
        fclose(file);
    }
}

void get_save_map_2d() {
    cv::Mat map1;
    int w, h;

    if (camera_name == "front" || camera_name == "back") {   // 800 * 1080
        cv::warpPerspective(undistort_maps[0], project_matrix, dst_resolution, map1);
        w = settings.total_w;
        h = settings.total_h;
    } else {   // 1080 * 800
        cv::warpPerspective(undistort_maps[0], project_matrix, dst_resolution_s, map1);
        w = settings.total_h;
        h = settings.total_w;
    }

    cv::Mat nearest(1920, 1080, CV_32FC2, cv::Scalar(0));
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            float x = map1.at<cv::Vec2f>(j, i)[0];
            float y = map1.at<cv::Vec2f>(j, i)[1];
            if (0 < x && x < 1920 && 0 < y && y < 1080) {
                nearest.at<cv::Vec2f>(i, j)[0] = y * 1920 + x;
                if (camera_name == "front") {
                    nearest.at<cv::Vec2f>(i, j)[1] = j * 800 + i;
                } else if (camera_name == "back") {
                    nearest.at<cv::Vec2f>(i, j)[1] = (1080 - j - 1) * 800 + (800 - i - 1);
                } else if (camera_name == "right") {
                    nearest.at<cv::Vec2f>(i, j)[1] = j + (1080 - i - 1) * 800;
                } else if (camera_name == "left") {
                    nearest.at<cv::Vec2f>(i, j)[1] = (800 - j) + i * 800;
                }
            }
        }
    }

    std::ofstream file("maps/near_01_" + camera_name + ".bin", std::ios::binary);
    file.write(reinterpret_cast<const char*>(nearest.data), 1920 * 1080 * sizeof(cv::Vec2f));
    file.close();
}
