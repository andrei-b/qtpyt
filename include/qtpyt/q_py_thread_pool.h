#pragma once

#include "qpymodule.h"
#include "q_py_future.h"
#include <atomic>
#include <functional>
#include <future>
#include <memory>
#include <thread>
#include <vector>

class QPyQueue;
namespace qtpyt {

    class QPyThreadPool {
    public:
        static void initialize(size_t threadCount = std::thread::hardware_concurrency(), bool useSubInterpreters = true);
        static QPyThreadPool& instance();
        static void saveMainThreadState();
        static void restoreMainThreadState();
        QPyThreadPool(const QPyThreadPool&) = delete;
        QPyThreadPool& operator=(const QPyThreadPool&) = delete;
        QPyThreadPool(const QPyThreadPool&&) = delete;
        QPyThreadPool& operator=(const QPyThreadPool&&) = delete;

        ~QPyThreadPool();

        void submit(QPyFuture future);

        void shutdown();

    private:
        explicit QPyThreadPool(size_t threadCount, bool useSubInterpreters);
        std::vector<std::thread> workers_;
        std::vector<std::unique_ptr<QPyQueue>> tasks_;
        std::atomic<bool> stop_;
        bool m_initialized{false};
        bool m_subInterpretersUsed;
        std::unordered_map<QPyModule*, int> m_threadCallableMap_;
    };
}// namespace qtpyt