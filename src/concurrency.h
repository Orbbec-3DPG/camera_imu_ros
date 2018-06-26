#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <functional>

const int QUEUE_MAX_SIZE = 10;

template<class T>

class single_consumer_queue
{
    std::deque<T> q;
    std::mutex mutex;
    std::condition_variable cv;
    unsigned int cap;
    bool accepting;

    std::atomic<bool> need_to_flush;
    std::atomic<bool> was_flushed;
    std::condition_variable was_flushed_cv;
    std::mutex was_flushed_mutex;
public:
    explicit single_consumer_queue<T>(unsigned int cap = QUEUE_MAX_SIZE)
        : q(), mutex(), cv(), cap(cap), need_to_flush(false), was_flushed(false), accepting(true)
    {}

    void enqueue(T&& item)
    {
        std::unique_lock<std::mutex> lock(mutex);
        if(accepting)
        {
            q.push_back(std::move(item));
            if(q.size() > cap)
            {
                q.pop_front();
            }
        }
        lock.unlock();
        cv.notify_one();
    }

    void dequeue(T* item, unsigned int timeout_ms = 5000)
    {
        std::unique_lock<std::mutex> lock(mutex);
        accepting = true;
        was_flushed = false;
        const auto ready = [this]() {return (q.size()>0) || need_to_flush;};
        if(!ready() && !cv.wait_for(lock, std::chrono::milliseconds(timeout_ms),ready))
        {
            return false;
        }

        if (q.size() <= 0)
        {
            return false;
        }

        *item =std::move(q.front());
        q.pop_front();
        return true;
    }

    bool peek(T** item)
    {
        std::unique_lock<std::mutex> lock(mutex);

        if (q.size() <= 0)
        {
            return false;
        }

        *item = &q.front();
        return true;
    }

    bool try_dequeue(T* item)
    {
        std::unique_lock<std::mutex> lock(mutex);
        accepting = true;
        if (q.size() > 0)
        {
            auto val = std::move(q.front());
            q.pop_front();
            *item = std::move(val);
            return true;
        }
        return false;
    }

    void clear()
    {
        std::unique_lock<std::mutex> lock(mutex);
        accepting = false;
        need_to_flush = true;

        while (q.size() >0 )
        {
            auto item = std::move(q.front());
            q.pop_front();
        }
        cv.notify_all();
    }

    void start()
    {
        std::unique_lock<std::mutex> lock(mutex);
        need_to_flush = false;
        accepting = true;
    }

    size_t size()
    {
        std::unique_lock<std::mutex> lock(mutex);
        return q.size();
    }
};










