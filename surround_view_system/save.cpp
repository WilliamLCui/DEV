#include <iostream>
#include <deque>
#include <vector>

// Assuming the necessary dependencies and functions are included

std::vector<std::deque<int>> get_lists_coords_3d_to_2d(std::string camera_name) {
    // 按输出像素数量建立列表，每个列表元素为一个堆栈，将原图二维坐标转换为列表下标，将其对应的最终输出坐标（即经过3D变化的屏幕坐标）放入堆栈

    // h = settings3d.out_height
    // w = settings3d.out_width

    int h = settings.total_h;
    int w = settings.total_w;

    // 创建一个空的堆栈
    std::deque<int> stack;

    // 创建一个 h * w 的二维列表，每个列表元素为空堆栈
    std::vector<std::deque<int>> list_of_stacks(h * w, stack);

    // 读取3D顶点到屏幕二维坐标的映射表
    std::vector<int> map = read_map(settings3d.maps_root_3d + camera_name + ".bin", np.int32);

    // blank_map = []

    std::cout << "getting lists of coords ..." << std::endl;
    for (int i = 0; i < map.size(); i += 4) {
        int oy = map[i];
        int ox = map[i + 1];
        int px = map[i + 2];
        int py = map[i + 3];
        // blank_img[oy, ox] = img[py, px]
        // blank_map[py, px] = [ox, oy]
        int src = py * w + px;
        int dst = oy * w + ox;
        list_of_stacks[src].push_back(dst);
    }

    return list_of_stacks;
}

#include <iostream>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp> // Include the necessary OpenCV headers

class FisheyeCameraModel {
    // Define the FisheyeCameraModel class as per your requirements
    // ...
};

class BirdView {
    // Define the BirdView class as per your requirements
    // ...
};

void get_weights() {
    std::vector<std::string> names = settings.camera_names;
    std::vector<std::string> images;
    std::vector<std::string> yamls;
    std::vector<FisheyeCameraModel> camera_models;

    for (const std::string& name : names) {
        std::string image_file = std::string(os.getcwd()) + std::string(settings.images_root) + name + ".jpg";
        std::string yaml_file = std::string(os.getcwd()) + "yaml/" + name + ".yaml";
        images.push_back(image_file);
        yamls.push_back(yaml_file);
        FisheyeCameraModel camera(camera_file, camera_name);
        camera_models.push_back(camera);
    }

    std::vector<cv::Mat> projected;
    for (int i = 0; i < images.size(); i++) {
        cv::Mat img = cv::imread(images[i]);

        // cv::imshow("img", img);
        // cv::waitKey(30000);
        img = camera_models[i].undistort(img);
        // cv::imshow("undistort", img);
        // cv::waitKey(30000);
        img = camera_models[i].project(img);
        // cv::imshow("project", img);
        // cv::waitKey(30000);
        img = camera_models[i].flip(img);
        // cv::imshow("flip", img);
        // cv::waitKey(30000);
        projected.push_back(img);
    }

    // for (const auto& img : projected) {
    //     cv::imshow("img", img);
    //     cv::waitKey(3000);
    // }

    BirdView birdview;
    cv::Mat Gmat, Mmat;
    birdview.get_weights_and_masks(projected, Gmat, Mmat);
    birdview.save_weights();
}
