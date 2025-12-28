#pragma once

#include <qtpyt/qpymodule.h>
#include <qtpyt/q_py_future.h>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>

class QPyQueue {
public:
    QPyQueue() = default;
    QPyQueue(const QPyQueue&) = delete;
    QPyQueue& operator=(const QPyQueue&) = delete;

    void push(qtpyt::QPyFuture&& item) {
        {
            std::lock_guard<std::mutex> lk(m_);
            q_.push_back(std::move(item));
        }
        cv_.notify_one();
    }

    std::optional<qtpyt::QPyFuture> wait_and_pop() {
        if (PyGILState_Check()) {
            qWarning() << "QPyQueue::wait_and_pop called while GIL is held! This may lead to deadlocks.";
        }
        std::unique_lock<std::mutex> lk(m_);
        cv_.wait(lk, [this]{ return !q_.empty() || closed_; });
        if (q_.empty()) {
            // c{losed_ && no items

            return std::nullopt;
        }
        qtpyt::QPyFuture item = std::move(q_.front());
        q_.pop_front();
        return item;
    }

    std::optional< qtpyt::QPyFuture> try_pop() {
        std::lock_guard<std::mutex> lk(m_);
        if (q_.empty()) return std::nullopt;
         qtpyt::QPyFuture item = std::move(q_.front());
        q_.pop_front();
        return item;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lk(m_);
        return q_.empty();
    }

    void close() {
        {
            std::lock_guard<std::mutex> lk(m_);
            closed_ = true;
        }
        cv_.notify_all();
    }

private:
    mutable std::mutex m_;
    std::condition_variable cv_;
    std::deque< qtpyt::QPyFuture> q_;
    bool closed_ = false;
};