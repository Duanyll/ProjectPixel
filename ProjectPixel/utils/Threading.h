#pragma once

#include "pch.h"

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

class InputForwarder {
   public:
    inline void add_time(const std::string& command, float time) {
        std::lock_guard<std::mutex> lg(mtx);
        keyTime[command] += time;
    }

    inline void add_event(const std::string& command) {
        std::lock_guard<std::mutex> lg(mtx);
        keyEvents.push(command);
    }

    inline void collect(std::unordered_map<std::string, float>& outKeyTime,
                        std::queue<std::string> outKeyEvents) {
        std::lock_guard<std::mutex> lg(mtx);
        for (auto& i : keyTime) {
            outKeyTime[i.first] += i.second;
            i.second = 0;
        }
        while (!keyEvents.empty()) {
            outKeyEvents.push(keyEvents.front());
            keyEvents.pop();
        }
    }

   private:
    mutable std::mutex mtx;
    std::unordered_map<std::string, float> keyTime;
    std::queue<std::string> keyEvents;
};

template <typename T>
class OutputBuffer {
   public:
    void update(std::unique_ptr<T>& ptr) { 
        std::lock_guard<std::mutex> lg(mtx);
        obj = std::move(ptr);
    }

    std::unique_ptr<T> try_get() { 
        std::lock_guard<std::mutex> lg(mtx);
        return std::move(obj);
    }

   private:
    mutable std::mutex mtx;
    std::unique_ptr<T> obj;
};

class WorkerThread {
   public:
    virtual std::chrono::milliseconds get_cycle_time() = 0;
    inline virtual void start() {
        if (isRunning) return;
        std::thread worker([this]() -> void {
            isRunning = true;
            while (!shouldStop) {
                auto finish_time =
                    std::chrono::steady_clock::now() + get_cycle_time();
                work();
                if (std::chrono::steady_clock::now() < finish_time) {
                    std::this_thread::sleep_until(finish_time);
                }
            }
            isRunning = false;
            shouldStop = false;
            condStop.notify_all();
        });
        worker.detach();
    }

    inline virtual void stop() {
        if (!isRunning) return;
        shouldStop = true;

        std::unique_lock<std::mutex> lock(mtxStop);
        condStop.wait(lock, [this] { return !isRunning; });
    }

   protected:
    virtual void work() = 0;
    mutable std::atomic<bool> shouldStop = false;
    mutable std::atomic<bool> isRunning = false;

    mutable std::mutex mtxStop;
    std::condition_variable condStop;
};