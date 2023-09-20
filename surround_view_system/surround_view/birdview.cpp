#include <iostream>
#include <vector>
#include <mutex>
#include <condition_variable>

#include <opencv2/opencv.hpp>

struct ProjectedImageBuffer {
    bool drop_if_full;
    Buffer buffer;
    std::set<int> sync_devices;
    std::condition_variable wc;
    std::mutex mutex;
    int arrived;
    std::map<int, cv::Mat> current_frames;

    ProjectedImageBuffer(bool drop_if_full = true, int buffer_size = 8)
            : drop_if_full(drop_if_full),
              buffer(buffer_size),
              arrived(0) {}

    void bind_thread(BaseThread* thread) {
        std::lock_guard<std::mutex> lock(mutex);
        sync_devices.insert(thread->device_id);

        std::string name = thread->camera_model.camera_name;
        cv::Size shape = settings::project_shapes[name];
        current_frames[thread->device_id] = cv::Mat::zeros(shape.width, shape.height, CV_8UC3);
        thread->proc_buffer_manager = this;
    }

    cv::Mat get() {
        return buffer.get();
    }

    void set_frame_for_device(int device_id, const cv::Mat& frame) {
        std::lock_guard<std::mutex> lock(mutex);

        if (sync_devices.find(device_id) == sync_devices.end()) {
            throw std::runtime_error("Device not held by the buffer: " + std::to_string(device_id));
        }

        current_frames[device_id] = frame;
    }
};


struct ProjectedImageBuffer {
    bool drop_if_full;
    Buffer buffer;
    std::set<int> sync_devices;
    std::condition_variable wc;
    std::mutex mutex;
    int arrived;
    std::map<int, cv::Mat> current_frames;

    ProjectedImageBuffer(bool drop_if_full = true, int buffer_size = 8)
            : drop_if_full(drop_if_full),
              buffer(buffer_size),
              arrived(0) {}

    void bind_thread(BaseThread* thread) {
        std::lock_guard<std::mutex> lock(mutex);
        sync_devices.insert(thread->device_id);

        std::string name = thread->camera_model.camera_name;
        cv::Size shape = settings::project_shapes[name];
        current_frames[thread->device_id] = cv::Mat::zeros(shape.width, shape.height, CV_8UC3);
        thread->proc_buffer_manager = this;
    }

    cv::Mat get() {
        return buffer.get();
    }

    void set_frame_for_device(int device_id, const cv::Mat& frame) {
        std::lock_guard<std::mutex> lock(mutex);

        if (sync_devices.find(device_id) == sync_devices.end()) {
            throw std::runtime_error("Device not held by the buffer: " + std::to_string(device_id));
        }

        current_frames[device_id] = frame;
    }

    void sync(int device_id) {
        std::unique_lock<std::mutex> lock(mutex);
        if (sync_devices.find(device_id) != sync_devices.end()) {
            arrived++;

            if (arrived == sync_devices.size()) {
                buffer.add(current_frames, drop_if_full);
                wc.notify_all();
            } else {
                wc.wait(lock);
            }

            arrived--;
        }
    }

    void wake_all() {
        std::unique_lock<std::mutex> lock(mutex);
        wc.notify_all();
    }

    bool contains(int device_id) {
        std::unique_lock<std::mutex> lock(mutex);
        return sync_devices.find(device_id) != sync_devices.end();
    }

    std::string toString() {
        std::unique_lock<std::mutex> lock(mutex);
        std::string result = "ProjectedImageBuffer:\n";
        result += "devices: ";
        for (int device_id : sync_devices) {
            result += std::to_string(device_id) + ", ";
        }
        result += "\n";
        return result;
    }
};

cv::Mat FI(const cv::Mat& front_image) {
    return front_image(cv::Range::all(), cv::Range(0, xl));
}

cv::Mat FII(const cv::Mat& front_image, int xr) {
    return front_image(cv::Range::all(), cv::Range(xr, front_image.cols));
}

cv::Mat FM(const cv::Mat& front_image, int xl, int xr) {
    return front_image(cv::Range::all(), cv::Range(xl, xr));
}

cv::Mat BIII(const cv::Mat& back_image, int xl) {
    return back_image(cv::Range::all(), cv::Range(0, xl));
}

cv::Mat BIV(const cv::Mat& back_image, int xr) {
    return back_image(cv::Range::all(), cv::Range(xr, back_image.cols));
}

cv::Mat BM(const cv::Mat& back_image, int xl, int xr) {
    return back_image(cv::Range::all(), cv::Range(xl, xr));
}

cv::Mat LI(const cv::Mat& left_image, int yt) {
    return left_image(cv::Range(0, yt), cv::Range::all());
}

cv::Mat LIII(const cv::Mat& left_image, int yb) {
    return left_image(cv::Range(yb, left_image.rows), cv::Range::all());
}

cv::Mat LM(const cv::Mat& left_image, int yt, int yb) {
    return left_image(cv::Range(yt, yb), cv::Range::all());
}

cv::Mat RII(const cv::Mat& right_image, int yt) {
    return right_image(cv::Range(0, yt), cv::Range::all());
}

cv::Mat RIV(const cv::Mat& right_image, int yb) {
    return right_image(cv::Range(yb, right_image.rows), cv::Range::all());
}

cv::Mat RM(const cv::Mat& right_image, int yt, int yb) {
    return right_image(cv::Range(yt, yb), cv::Range::all());
}

class BirdView : public BaseThread {
private:
    ProcBufferManager* proc_buffer_manager;
    bool drop_if_full;
    Buffer buffer;
    cv::Mat image;
    std::vector<cv::Mat> weights;
    std::vector<cv::Mat> masks;
    std::string car_image;
    std::vector<cv::Mat> frames;

public:
    BirdView(ProcBufferManager* proc_buffer_manager = nullptr,
             bool drop_if_full = true,
             int buffer_size = 8,
             BaseThread* parent = nullptr) : BaseThread(parent),
                                             proc_buffer_manager(proc_buffer_manager),
                                             drop_if_full(drop_if_full),
                                             buffer(buffer_size) {
        image = cv::Mat::zeros(settings.total_h, settings.total_w, CV_8UC3);
        weights.resize(4);
        masks.resize(4);
        car_image = settings.car_image;
        frames.clear();
    }

    cv::Mat get() {
        return buffer.get();
    }

    void update_frames(std::vector<cv::Mat> images) {
        frames = images;
    }

    void load_weights_and_masks(const std::string& weights_image, const std::string& masks_image) {
        cv::Mat GMat = cv::imread(weights_image, cv::IMREAD_UNCHANGED);
        GMat.convertTo(GMat, CV_32F, 1.0 / 255.0);
        cv::split(GMat, weights);

        cv::Mat Mmat = cv::imread(masks_image, cv::IMREAD_UNCHANGED);
        utils::convert_binary_to_bool(Mmat, masks);
    }

    cv::Mat merge(const cv::Mat& imA, const cv::Mat& imB, int k) {
        cv::Mat G = weights[k];
        cv::Mat merged = imA.mul(G) + imB.mul(1 - G);
        return merged;
    }

    cv::Mat FL() {
        return image(cv::Range(0, yt), cv::Range(0, xl));
    }

    cv::Mat F() {
        return image(cv::Range(0, yt), cv::Range(xl, xr));
    }

    cv::Mat FR() {
        return image(cv::Range(0, yt), cv::Range(xr, image.cols));
    }

    cv::Mat BL() {
        return image(cv::Range(yb, image.rows), cv::Range(0, xl));
    }

    cv::Mat B() {
        return image(cv::Range(yb, image.rows), cv::Range(xl, xr));
    }

    cv::Mat BR() {
        return image(cv::Range(yb, image.rows), cv::Range(xr, image.cols));
    }

    cv::Mat L() {
        return image(cv::Range(yt, yb), cv::Range(0, xl));
    }

    cv::Mat R() {
        return image(cv::Range(yt, yb), cv::Range(xr, image.cols));
    }

    cv::Mat C() {
        return image(cv::Range(yt, yb), cv::Range(xl, xr));
    }
};

void stitch_all_parts() {
    cv::Mat front, back, left, right;
    front = frames[0];
    back = frames[1];
    left = frames[2];
    right = frames[3];

    cv::Mat F, B, L, R, FL, BL, FR, BR;

    FM(front).copyTo(F);
    BM(back).copyTo(B);
    LM(left).copyTo(L);
    RM(right).copyTo(R);

    merge(FI(front), LI(left), 0).copyTo(FL);
    merge(BIII(back), LIII(left), 1).copyTo(BL);
    merge(FII(front), RII(right), 2).copyTo(FR);
    merge(BIV(back), RIV(right), 3).copyTo(BR);
}

void copy_car_image() {
    car_image.copyTo(C);
}

void make_luminance_balance() {
    auto tune = [](float x) {
        if (x >= 1) {
            return x * exp((1 - x) * 0.5);
        } else {
            return x * exp((1 - x) * 0.8);
        }
    };

    cv::Mat front, back, left, right;
    front = frames[0];
    back = frames[1];
    left = frames[2];
    right = frames[3];

    cv::Mat m1, m2, m3, m4;
    m1 = masks[0];
    m2 = masks[1];
    m3 = masks[2];
    m4 = masks[3];

    std::vector<cv::Mat> Fchannels, Bchannels, Lchannels, Rchannels;
    cv::split(front, Fchannels);
    cv::split(back, Bchannels);
    cv::split(left, Lchannels);
    cv::split(right, Rchannels);

    cv::Mat Fb = Fchannels[0], Fg = Fchannels[1], Fr = Fchannels[2];
    cv::Mat Bb = Bchannels[0], Bg = Bchannels[1], Br = Bchannels[2];
    cv::Mat Lb = Lchannels[0], Lg = Lchannels[1], Lr = Lchannels[2];
    cv::Mat Rb = Rchannels[0], Rg = Rchannels[1], Rr = Rchannels[2];

    float a1, a2, a3, b1, b2, b3, c1, c2, c3, d1, d2, d3, t1, t2, t3, x1, x2;

    a1 = utils::mean_luminance_ratio(RII(Rb), FII(Fb), m3);
    a2 = utils::mean_luminance_ratio(RII(Rg), FII(Fg), m3);
    a3 = utils::mean_luminance_ratio(RII(Rr), FII(Fr), m3);
    b1 = utils::mean_luminance_ratio(BIV(Bb), RIV(Rb), m4);
    b2 = utils::mean_luminance_ratio(BIV(Bg), RIV(Rg), m4);
    b3 = utils::mean_luminance_ratio(BIV(Br), RIV(Rr), m4);
    c1 = utils::mean_luminance_ratio(LIII(Lb), BIII(Bb), m2);
    c2 = utils::mean_luminance_ratio(LIII(Lg), BIII(Bg), m2);
    c3 = utils::mean_luminance_ratio(LIII(Lr), BIII(Br), m2);
    d1 = utils::mean_luminance_ratio(FI(Fb), LI(Lb), m1);
    d2 = utils::mean_luminance_ratio(FI(Fg), LI(Lg), m1);
    d3 = utils::mean_luminance_ratio(FI(Fr), LI(Lr), m1);

    t1 = pow(a1 * b1 * c1 * d1, 0.25);
    t2 = pow(a2 * b2 * c2 * d2, 0.25);
    t3 = pow(a3 * b3 * c3 * d3, 0.25);

    x1 = t1 / sqrt(d1 / a1);
    x2 = tune(x2);
    x3 = tune(x3);
    Fb = utils.adjust_luminance(Fb, x1);
    Fg = utils.adjust_luminance(Fg, x2);
    Fr = utils.adjust_luminance(Fr, x3);
    y1 = t1 / pow(b1 / c1, 0.5);
    y2 = t2 / pow(b2 / c2, 0.5);
    y3 = t3 / pow(b3 / c3, 0.5);
    y1 = tune(y1);
    y2 = tune(y2);
    y3 = tune(y3);
    Bb = utils.adjust_luminance(Bb, y1);
    Bg = utils.adjust_luminance(Bg, y2);
    Br = utils.adjust_luminance(Br, y3);
    z1 = t1 / pow(c1 / d1, 0.5);
    z2 = t2 / pow(c2 / d2, 0.5);
    z3 = t3 / pow(c3 / d3, 0.5);
    z1 = tune(z1);
    z2 = tune(z2);
    z3 = tune(z3);
    Lb = utils.adjust_luminance(Lb, z1);
    Lg = utils.adjust_luminance(Lg, z2);
    Lr = utils.adjust_luminance(Lr, z3);
    w1 = t1 / pow(a1 / b1, 0.5);
    w2 = t2 / pow(a2 / b2, 0.5);
    w3 = t3 / pow(a3 / b3, 0.5);
    w1 = tune(w1);
    w2 = tune(w2);
    w3 = tune(w3);
    Rb = utils.adjust_luminance(Rb, w1);
    Rg = utils.adjust_luminance(Rg, w2);
    Rr = utils.adjust_luminance(Rr, w3);
    frames = {cv::merge({Fb, Fg, Fr}),
              cv::merge({Bb, Bg, Br}),
              cv::merge({Rb, Rg, Rr}),
              cv::merge({Lb, Lg, Lr})};
    return frames;
}

void get_weights_and_masks(std::vector<cv::Mat>& images) {
    cv::Mat front = images[0];
    cv::Mat back = images[1];
    cv::Mat left = images[2];
    cv::Mat right = images[3];

    cv::Mat G0, M0;
    utils::get_weight_mask_matrix(FI(front), LI(left), G0, M0);

    cv::Mat G1, M1;
    utils::get_weight_mask_matrix(BIII(back), LIII(left), G1, M1);

    cv::Mat G2, M2;
    utils::get_weight_mask_matrix(FII(front), RII(right), G2, M2);

    cv::Mat G3, M3;
    utils::get_weight_mask_matrix(BIV(back), RIV(right), G3, M3);

    self.weights.clear();
    self.weights.push_back(cv::merge(std::vector<cv::Mat>{G0, G0, G0}));
    self.weights.push_back(cv::merge(std::vector<cv::Mat>{G1, G1, G1}));
    self.weights.push_back(cv::merge(std::vector<cv::Mat>{G2, G2, G2}));
    self.weights.push_back(cv::merge(std::vector<cv::Mat>{G3, G3, G3}));

    self.masks.clear();
    self.masks.push_back((M0 / 255.0).astype(np.int));
    self.masks.push_back((M1 / 255.0).astype(np.int));
    self.masks.push_back((M2 / 255.0).astype(np.int));
    self.masks.push_back((M3 / 255.0).astype(np.int));

    return cv::merge(std::vector<cv::Mat>{G0, G1, G2, G3}), cv::merge(std::vector<cv::Mat>{M0, M1, M2, M3});
}

void get_weights_and_masks_td(std::vector<cv::Mat>& images) {
    cv::Mat front = images[0];
    cv::Mat back = images[1];
    cv::Mat left = images[2];
    cv::Mat right = images[3];

    cv::Mat G0, M0;
    utils::get_weight_mask_matrix1(front(cv::Rect(0, 0, xl, yt)), left(cv::Rect(0, 0, xl, yt)), G0, M0);

    cv::Mat G1, M1;
    utils::get_weight_mask_matrix1(back(cv::Rect(0, yb, xl, back.rows - yb)), left(cv::Rect(0, yb, xl, left.rows - yb)), G1, M1);

    cv::Mat G2, M2;
    utils::get_weight_mask_matrix1(front(cv::Rect(xr, 0, front.cols - xr, yt)), right(cv::Rect(xr, 0, right.cols - xr, yt)), G2, M2);

    cv::Mat G3, M3;
    utils::get_weight_mask_matrix1(back(cv::Rect(xr, yb, back.cols - xr, back.rows - yb)), right(cv::Rect(xr, yb, right.cols - xr, right.rows - yb)), G3, M3);

    self.weights.clear();
    self.weights.push_back(cv::merge(std::vector<cv::Mat>{G0, G0, G0}));
    self.weights.push_back(cv::merge(std::vector<cv::Mat>{G1, G1, G1}));
    self.weights.push_back(cv::merge(std::vector<cv::Mat>{G2, G2, G2}));
    self.weights.push_back(cv::merge(std::vector<cv::Mat>{G3, G3, G3}));

    self.masks.clear();
    self.masks.push_back((M0 / 255.0).astype(np.int));
    self.masks.push_back((M1 / 255.0).astype(np.int));
    self.masks.push_back((M2 / 255.0).astype(np.int));
    self.masks.push_back((M3 / 255.0).astype(np.int));

    return cv::merge(std::vector<cv::Mat>{G0, G1, G2, G3}), cv::merge(std::vector<cv::Mat>{M0, M1, M2, M3});
}

void save_weights() {
    for (int i = 0; i < 4; ++i) {
        std::ofstream file("maps/weight_" + std::to_string(i) + ".txt", std::ios::binary);
        cv::Mat weightsFlatten;
        self.weights[i].reshape(1, 1).convertTo(weightsFlatten, CV_32F);
        file.write(reinterpret_cast<const char*>(weightsFlatten.data), weightsFlatten.total() * sizeof(float));
        file.close();
    }

    for (int i = 0; i < 4; ++i) {
        std::ofstream file("maps/nearest/weight_" + std::to_string(i) + ".txt", std::ios::binary);
        cv::Mat weightsFlatten;
        self.weights[i].reshape(1, 1).convertTo(weightsFlatten, CV_32F);
        file.write(reinterpret_cast<const char*>(weightsFlatten.data), weightsFlatten.total() * sizeof(float));
        file.close();
    }
}

void make_white_balance() {
    self.image = utils::make_white_balance(self.image);
}

void run() {
    if (proc_buffer_manager == nullptr) {
        throw std::invalid_argument("This thread requires a buffer of projected images to run");
    }

    while (true) {
        stop_mutex.lock();
        if (stopped) {
            stopped = false;
            stop_mutex.unlock();
            break;
        }
        stop_mutex.unlock();

        processing_time = clock.elapsed();
        clock.start();

        processing_mutex.lock();

        update_frames(proc_buffer_manager->get()->values());
        make_luminance_balance().stitch_all_parts();
        make_white_balance();
        copy_car_image();
        buffer.add(image.copy(), drop_if_full);
        processing_mutex.unlock();


        update_fps(processing_time);
        stat_data.frames_processed_count += 1;

        update_statistics_gui.emit(stat_data);
    }
}
