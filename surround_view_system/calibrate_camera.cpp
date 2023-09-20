#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

void calib(cv::Size inter_corner_shape, double size_per_grid, std::string path) {
    // 获取棋盘格角点个数
    int w = inter_corner_shape.width;
    int h = inter_corner_shape.height;

    // 世界坐标维度为三，与角点维度对齐
    // 普通相机校正中世界坐标比角点坐标少一个维，在鱼眼相机校正中少一维会报错
    cv::Mat cp_int = cv::Mat::zeros(1, w * h, CV_32FC3);
    cv::Mat cp_world = cp_int.reshape(3) * size_per_grid;

    // 检测亚像素角点
    cv::TermCriteria subpix_criteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 30, 0.1);

    // 标定方法
    int calibration_flags = cv::fisheye::CALIB_RECOMPUTE_EXTRINSIC + cv::fisheye::CALIB_CHECK_COND + cv::fisheye::CALIB_FIX_SKEW;

    // 图像尺寸
    cv::Size img_shape;

    // 世界坐标
    std::vector<cv::Mat> obj_points;

    // 角点坐标
    std::vector<cv::Mat> img_points;

    // 获取图片
    std::vector<std::string> images;
    cv::glob(path + "/*.jpg", images, false);

    for (const std::string& fname : images) {
        cv::Mat img = cv::imread(fname);

        // 使用第一张图片得到图像尺寸
        if (img_shape.width == 0 && img_shape.height == 0) {
            img_shape = img.size();
        } else {
            assert(img_shape == img.size());
        }

        cv::Mat gray_img;
        cv::cvtColor(img, gray_img, cv::COLOR_BGR2GRAY);

        // 找角点
        bool ret = cv::findChessboardCorners(gray_img, inter_corner_shape, corners,
                                             cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_FAST_CHECK + cv::CALIB_CB_NORMALIZE_IMAGE);

        if (ret) {
            std::cout << fname << std::endl;
            cv::drawChessboardCorners(img, inter_corner_shape, corners, ret);
            cv::imshow(fname, img);
            cv::waitKey(0);

            obj_points.push_back(cp_world);

            // 添加亚像素角点信息
            cv::cornerSubPix(gray_img, corners, cv::Size(3, 3), cv::Size(-1, -1), subpix_criteria);
            img_points.push_back(corners);
        }
    }

    // 实际图片数
    int imgs = obj_points.size();

    // 初始化内参、畸变系数以及旋转和平移向量
    cv::Mat K = cv::Mat::zeros(3, 3, CV_64FC1);
    cv::Mat D = cv::Mat::zeros(4, 1, CV_64FC1);
    std::vector<cv::Mat> rvecs(imgs, cv::Mat::zeros(1, 1, CV_64FC3));
}


cv::Mat superundistort(const std::string& path, cv::Mat K, cv::Mat D, cv::Size DIM, double scale = 0.6, bool imshow = true) {
    cv::Mat img = cv::imread(path);
    cv::Size dim1 = img.size();  // dim1 is the dimension of input image to un-distort
    assert(dim1.width / dim1.height == DIM.width / DIM.height, "Image to undistort needs to have same aspect ratio as the ones used in calibration");
    if (dim1.width != DIM.width) {
        cv::resize(img, img, DIM, 0, 0, cv::INTER_AREA);
    }
    cv::Mat NewK = K.clone();
    if (scale) {
        NewK.at<double>(0, 0) = scale * NewK.at<double>(0, 0);
        NewK.at<double>(1, 1) = scale * NewK.at<double>(1, 1);
    }
    cv::Mat map1, map2;
    cv::fisheye::initUndistortRectifyMap(K, D, cv::Mat::eye(3, 3, CV_32F), NewK, DIM, CV_16SC2, map1, map2);
    cv::Mat undistorted_img;
    cv::remap(img, undistorted_img, map1, map2, cv::INTER_LINEAR, cv::BORDER_CONSTANT);
    if (imshow) {
        cv::namedWindow("distorted_img", cv::WINDOW_NORMAL);
        cv::namedWindow("undistorted_img", cv::WINDOW_NORMAL);
        cv::imshow("distorted_img", img);
        cv::imshow("undistorted_img", undistorted_img);
        cv::waitKey(0);
        cv::destroyAllWindows();
    }
    else {
        return undistorted_img;
    }
}

int main() {
    cv::Size inter_corner_shape(9, 6);
    double size_per_grid = 0.02;
    cv::Size DIM;
    cv::Mat K, D;
    calib(inter_corner_shape, size_per_grid, "capture_images/guangzhou", DIM, K, D);
    cv::Mat img = superundistort("capture_images/guangzhou/output_1.jpg", K, D, DIM);

    return 0;
}
