#include <opencv2/opencv.hpp>
#include <QDebug>
#include "base_thread.h"
#include "image_frame.h"
#include "utils.h"

class CaptureThread : public BaseThread {
public:
    CaptureThread(int device_id,
                  int flip_method=2,
                  bool drop_if_full=true,
                  int api_preference=cv2::CAP_GSTREAMER,
                  cv::Size resolution=cv::Size(),
                  bool use_gst=true,
                  QObject *parent=nullptr)
            : BaseThread(parent),
              device_id(device_id),
              flip_method(flip_method),
              use_gst(use_gst),
              drop_if_full(drop_if_full),
              api_preference(api_preference),
              resolution(resolution),
              cap(cv::VideoCapture()),
              buffer_manager(nullptr)
    {
    }

    void run() {
        if (buffer_manager == nullptr) {
            throw std::invalid_argument("This thread has not been binded to any buffer manager yet");
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

            buffer_manager->sync(device_id);
        }
    }

private:
    int device_id;
    int flip_method;
    bool use_gst;
    bool drop_if_full;
    int api_preference;
    cv::Size resolution;
    cv::VideoCapture cap;
    MultiBufferManager* buffer_manager;
};

bool connect_camera() {
    if (use_gst) {
        std::string options = gstreamer_pipeline(cam_id, flip_method);
        cap.open(options, api_preference);
    }
    else {
        cap.open(device_id);
    }


    if (!cap.isOpened()) {
        qDebug("Cannot open camera {}", device_id);
        return false;
    }
    else {

        if (resolution != nullptr) {
            int width = resolution->first;
            int height = resolution->second;
            cap.set(cv2.CAP_PROP_FRAME_WIDTH, width);
            cap.set(cv2.CAP_PROP_FRAME_HEIGHT, height);

            if (!cap.isOpened()) {
                qDebug("Resolution not supported by camera device: {}", resolution);
                return false;
            }
        }

        else {
            int width = static_cast<int>(cap.get(cv2.CAP_PROP_FRAME_WIDTH));
            int height = static_cast<int>(cap.get(cv2.CAP_PROP_FRAME_HEIGHT));
            resolution = std::make_pair(width, height);
        }
    }

    return true;
}

bool disconnect_camera() {

    if (cap.isOpened()) {
        cap.release();
        return true;
    }

    else {
        return false;
    }
}

bool is_camera_connected() {
    return cap.isOpened();
}
