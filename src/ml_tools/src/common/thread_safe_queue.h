#ifndef THREAD_SAFE_QUEUE_H
#define THREAD_SAFE_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>


// All credits and complaints to: Claude 3.5 Sonnet

template <typename T>
class ThreadSafeQueue {
private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cond_;

public:
    ThreadSafeQueue() {}

    // Prevent copying
    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

    // Push an item to the queue
    void push(const T& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(value);
        cond_.notify_one();
    }

    void push(T&& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(value));
        cond_.notify_one();
    }

    // Blocking pop
    T wait_and_pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        // Wait until the queue is not empty
        cond_.wait(lock, [this]{ return !queue_.empty(); });
        
        T value = queue_.front();
        queue_.pop();
        return value;
    }

    // Try to pop with timeout (C++11 compatible)
    bool try_pop(T& value, unsigned long timeout_ms) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // Wait with timeout
        if (cond_.wait_for(lock, 
            std::chrono::milliseconds(timeout_ms), 
            [this]{ return !queue_.empty(); })) {
            value = queue_.front();
            queue_.pop();
            return true;
        }
        
        return false;
    }

    // Non-blocking try pop
    bool try_pop(T& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return false;
        }
        
        value = queue_.front();
        queue_.pop();
        return true;
    }

    // Check if queue is empty
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    // Get queue size
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

    // Clear the entire queue
    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        std::queue<T> empty;
        std::swap(queue_, empty);
    }
};

#endif // THREAD_SAFE_QUEUE_H