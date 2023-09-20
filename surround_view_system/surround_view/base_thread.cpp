#include <queue>
#include <mutex>
#include <chrono>
#include <iostream>

class BaseThread : public QThread {
public:
    BaseThread(QObject* parent = nullptr) : QThread(parent) {
        init_commons();
    }

    void stop() {
        std::lock_guard<std::mutex> lock(stop_mutex);
        stopped = true;
    }

    void update_fps(int dt) {
        if (dt > 0) {
            fps.push(1000 / dt);
        }

        if (fps.size() > FPS_STAT_QUEUE_LENGTH) {
            fps.pop();
        }

        if (fps.size() == FPS_STAT_QUEUE_LENGTH) {
            while (!fps.empty()) {
                fps_sum += fps.front();
                fps.pop();
            }

            stat_data.average_fps = round(fps_sum / FPS_STAT_QUEUE_LENGTH, 2);
            fps_sum = 0;
        }
    }


protected:
    void init_commons() {
        stopped = false;
        fps_sum = 0;
        QTime clock;
    }

    std::queue<int> fps;
    std::mutex stop_mutex;
    bool stopped;
    int fps_sum;
    const int FPS_STAT_QUEUE_LENGTH = 32;
    ThreadStatisticsData stat_data;
};

