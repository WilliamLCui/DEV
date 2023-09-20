class ImageFrame {
public:
    int timestamp;
    cv::Mat image;

    ImageFrame(int ts, cv::Mat img) : timestamp(ts), image(img) {}
};

class ThreadStatisticsData {
public:
    double average_fps;
    int frames_processed_count;

    ThreadStatisticsData() : average_fps(0), frames_processed_count(0) {}
};
