#include <opencv2/opencv.hpp>
#include <QMutex>
#include "base_thread.h"

class CameraProcessingThread : public BaseThread {
public:
    CameraProcessingThread(MultiBufferManager* capture_buffer_manager,
                           int device_id,
                           FisheyeCameraModel* camera_model,
                           bool drop_if_full = true,
                           QObject* parent = nullptr)
            : BaseThread(parent),
              capture_buffer_manager(capture_buffer_manager),
              device_id(device_id),
              camera_model(camera_model),
              drop_if_full(drop_if_full),
              proc_buffer_manager(nullptr) {
    }

    void run() override {
        if (proc_buffer_manager == nullptr) {
            throw std::runtime_error("This thread has not been bound to any processing thread yet");
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
            RawFrame raw_frame = capture_buffer_manager->getDevice(device_id)->get();
            cv::Mat und_frame = camera_model->undistort(raw_frame.image);
            cv::Mat pro_frame = camera_model->project(und_frame);
            cv::Mat flip_frame = camera_model->flip(pro_frame);
            processing_mutex.unlock();
            proc_buffer_manager->sync(device_id);
            proc_buffer_manager->setFrameForDevice(device_id, flip_frame);
        }
    }

private:
    MultiBufferManager* capture_buffer_manager;
    int device_id;
    FisheyeCameraModel* camera_model;
    bool drop_if_full;
    ProjectedImageBuffer* proc_buffer_manager;
};
