#pragma once

#include <chrono>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

template <typename T, typename TContainer = std::deque<T>>
class ConcurrenceQueue {
   public:
    ConcurrenceQueue() {}
    ConcurrenceQueue(const ConcurrenceQueue& other) {
        std::lock_guard<std::mutex> lg(other.mtx);
        queue = other.queue;
    }
    ConcurrenceQueue(ConcurrenceQueue&&) = delete;
    ConcurrenceQueue& operator=(const ConcurrenceQueue&) = delete;
    ~ConcurrenceQueue() = default;
    bool empty() const {
        std::lock_guard<std::mutex> lg(mtx);
        return queue.empty();
    }

    void push(const T& data) {
        std::lock_guard<std::mutex> lg(mtx);
        queue.push(data);
        cond.notify_one();
    }

    void push(T&& data) {
        std::lock_guard<std::mutex> lg(mtx);
        queue.push(std::move(data));
        cond.notify_one();
    }

    std::shared_ptr<T> pop_now() {
        std::lock_guard<std::mutex> lg(mtx);
        if (queue.empty()) return {};
        auto res = std::make_shared<T>(queue.front());
        queue.pop();
        return res;
    }

    std::shared_ptr<T> pop_wait() {
        std::unique_lock<std::mutex> lg(mtx);
        cond.wait(lg, [this] { return !queue.empty(); });
        auto res = std::make_shared<T>(std::move(queue.front()));
        queue.pop();
        return res;
    }

   private:
    std::queue<T, TContainer> queue;
    mutable std::mutex mtx;
    std::condition_variable cond;
};

typedef decltype(std::chrono::steady_clock::now()) TimeStamp;

class WorkerThread {
   public:
    virtual std::chrono::milliseconds get_cycle_time() = 0;
    inline virtual void start() {
        if (is_running) return;
        std::thread worker([this]() -> void {
            is_running = true;
            while (!stop_flag) {
                auto finish_time =
                    std::chrono::steady_clock::now() + get_cycle_time();
                work();
                if (std::chrono::steady_clock::now() < finish_time) {
                    std::this_thread::sleep_until(finish_time);
                }
            }
            is_running = false;
            stop_flag = false;
        });
        worker.detach();
    }

    inline virtual void stop() {
        if (!is_running) return;
        stop_flag = true;
        while (is_running)
            ;
    }

   protected:
    virtual void work() = 0;
    mutable std::atomic<bool> stop_flag = false;
    mutable std::atomic<bool> is_running = false;
};