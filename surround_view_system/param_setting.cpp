#include <iostream>
#include <string>

int main() {
    // 输出图片规格设定，暂无作用
    int out_width = 800;
    int out_height = 1080;

    // 2D
    std::string img_projected_root_2d = "results/projected_";
    std::string maps_root_2d = "maps/map_2d_";
    std::string img_warped_root_2d = "results/warped_2d_";
    std::string img_result_root_2d = "results/result_2d.jpg";

    // 3D
    std::string maps_root_3d = "maps/map_3d_";
    // 3D模型参数：
    // 图片规格1080p，一个棋盘格对应像素数 >9px
    // 收缩偏移量
    int shift = 200;
    // 边缘收缩速率
    int compress = 4;
    // 横向缩放倍数
    float x_scale = 1.6;
    // 纵向缩放倍数
    float y_scale = 1.6;
    // 半径大小为变成的 1/r_ratio
    int r_ratio = 4;

    // # 图片规格2k，一个棋盘格对应像素数 >6px
    // # 收缩偏移量
    // int shift = 330;
    // # 边缘收缩速率
    // int compress = 4;
    // # 横向缩放倍数
    // float x_scale = 5;
    // # 纵向缩放倍数
    // float y_scale = 5;
    // # 半径大小为变成的 1/r_ratio
    // int r_ratio = 17;

    // 窗口调试 1调试，0不调试 ， 由于窗口调试运行过慢，不建议用
    int test_loop_3d = 0;

    // 2d+3d文件路径
    std::string maps_root_final = "maps/nearest_";
    std::string img_warped_root_final = "results/warped_final_";
    std::string img_result_root_final = "results/result_final.jpg";

    // 最终参数
    std::string maps_root_nearest = "maps/nearest/nearest_";

    return 0;
}
