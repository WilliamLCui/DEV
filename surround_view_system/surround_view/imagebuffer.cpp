#include <QSemaphore>
#include <QMutex>
#include <QMutexLocker>
#include <queue>

class Buffer {
public:
    Buffer(int buffer_size = 5)
            : buffer_size(buffer_size), free_slots(buffer_size), used_slots(0),
              clear_buffer_add(1), clear_buffer_get(1) {}

    void add(const T& data, bool drop_if_full = false) {
        clear_buffer_add.acquire();
        if (drop_if_full) {
            if (free_slots.tryAcquire()) {
                QMutexLocker lock(&queue_mutex);
                queue.push(data);
                lock.unlock();
                used_slots.release();
            }
        } else {
            free_slots.acquire();
            QMutexLocker lock(&queue_mutex);
            queue.push(data);
            lock.unlock();
            used_slots.release();
        }
        clear_buffer_add.release();
    }

    T get() {
        clear_buffer_get.acquire();
        used_slots.acquire();
        QMutexLocker lock(&queue_mutex);
        T data = queue.front();
        queue.pop();
        lock.unlock();
        free_slots.release();
        clear_buffer_get.release();
        return data;
    }

    void clear() {
        if (!queue.empty()) {
            if (clear_buffer_add.tryAcquire()) {
                if (clear_buffer_get.tryAcquire()) {
                    free_slots.release(queue.size());
                }
            }
        }
    }

private:
    int buffer_size;
    QSemaphore free_slots;
    QSemaphore used_slots;
    QSemaphore clear_buffer_add;
    QSemaphore clear_buffer_get;
    QMutex queue_mutex;
    std::queue<T> queue;
};

#include <condition_variable>
#include <unordered_set>
#include <unordered_map>
#include <mutex>
#include <queue>

class Buffer {
private:
    std::queue<int> queue;
    int buffer_size;

public:
    Buffer(int size) : buffer_size(size) {}

    int size() {
        return queue.size();
    }

    int maxsize() {
        return buffer_size;
    }

    bool isfull() {
        return queue.size() == buffer_size;
    }

    bool isempty() {
        return queue.empty();
    }
};

class MultiBufferManager {
private:
    std::unordered_set<int> sync_devices;
    bool do_sync;
    std::condition_variable_any wc;
    std::mutex mutex;
    int arrived;
    std::unordered_map<int, Buffer*> buffer_maps;

public:
    MultiBufferManager(bool sync = true) : do_sync(sync), arrived(0) {}

    void bind_thread(Thread* thread, int buffer_size, bool sync = true) {
        create_buffer_for_device(thread->device_id, buffer_size, sync);
        thread->buffer_manager = this;
    }

    void create_buffer_for_device(int device_id, int buffer_size, bool sync = true) {
        if (sync) {
            std::lock_guard<std::mutex> lock(mutex);
            sync_devices.insert(device_id);
        }

        buffer_maps[device_id] = new Buffer(buffer_size);
    }

    Buffer* get_device(int device_id) {
        return buffer_maps[device_id];
    }

    void remove_device(int device_id) {
        delete buffer_maps[device_id];
        buffer_maps.erase(device_id);

        std::lock_guard<std::mutex> lock(mutex);
        if (sync_devices.count(device_id) > 0) {
            sync_devices.erase(device_id);
            wc.notify_all();
        }
    }

    void sync(int device_id) {
        std::unique_lock<std::mutex> lock(mutex);
        if (sync_devices.count(device_id) > 0) {
            arrived += 1;
            if (do_sync && arrived == sync_devices.size()) {
                wc.notify_all();
            } else {
                wc.wait(lock);
            }
            arrived -= 1;
        }
    }
};
