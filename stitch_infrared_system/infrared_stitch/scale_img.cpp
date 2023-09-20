//
// Created by Willi on 2023/8/20.
//
#include <vector>
#include <iostream>

struct Info {
    int h_img;
    int w_img;
};

std::pair<int, int> trans_point_dst2src(int u_dst, int v_dst, int h_src, int w_src, int h_dst, int w_dst) {
    int u_src = static_cast<int>(u_dst / (static_cast<float>(h_dst) / h_src));
    int v_src = static_cast<int>(v_dst / (static_cast<float>(w_dst) / w_src));
    return std::make_pair(u_src, v_src);
}

std::vector<std::vector<int>> map_stack(int h_src, int w_src, int h_dst, int w_dst) {
    std::vector<std::vector<int>> list_of_src(h_src * w_src);

    for (int u = 0; u < h_dst; ++u) {
        for (int v = 0; v < w_dst; ++v) {
            int dst = u * w_dst + v;

            std::pair<int, int> src = trans_point_dst2src(u, v, h_src, w_src, h_dst, w_dst);

            int src_index = src.first * w_src + src.second;

            list_of_src[src_index].push_back(dst);
        }
    }

    return list_of_src;
}

std::vector<int> map_trans_by_stack(const std::vector<int>& map_, const std::vector<std::vector<int>>& list_of_src) {
    std::vector<int> map_new;
    for (size_t i = 0; i < map_.size(); i += 2) {
        int src = map_[i];
        int dst1 = map_[i + 1];

        for (size_t j = 0; j < list_of_src[dst1].size(); ++j) {
            int dst2 = list_of_src[dst1][j];
            map_new.push_back(src);
            map_new.push_back(dst2);
        }
    }

    return map_new;
}

std::vector<int> scale_map(const Info& info, float ratio, const std::vector<int>& map) {
    std::cout << "Scaling map ..." << std::endl;
    std::vector<std::vector<int>> list_of_src = map_stack(info.h_img, info.w_img, static_cast<int>(info.h_img * ratio), static_cast<int>(info.w_img * ratio));

    std::vector<int> map_n = map_trans_by_stack(map, list_of_src);

    return map_n;
}