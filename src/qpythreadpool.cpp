#include <qtpyt/qpythreadpool.h>
#include "internal/q_py_sub_interpreter.h"
#include "internal/q_py_queue.h"

namespace qtpyt {
    static int _threadCount = 0;
    static bool _useSubInterpreters = false;
    static bool _initialized = false;
    static PyThreadState* _mainThreadState = nullptr;

    QPyThreadPool::QPyThreadPool(size_t threadCount, bool useSubInterpreters) : stop_(false) {
        if (threadCount == 0)
            threadCount = 1;
        workers_.reserve(threadCount);
        tasks_.resize(threadCount); // one queue per worker

        for (size_t i = 0; i < threadCount; ++i) {
            tasks_[i] = std::make_unique<::QPyQueue>();
            workers_.emplace_back([this, i] {
                while (!stop_.load()) {
                        auto taskOpt = tasks_[i]->wait_and_pop();
                        if (!taskOpt) {
                            continue;
                        }
                    {
                        pybind11::gil_scoped_acquire gil;
                        taskOpt.value()();
                    }
                }
            });
        }
    }

    void QPyThreadPool::initialize(size_t threadCount, bool useSubInterpreters) {
        _threadCount = threadCount;
        _useSubInterpreters = useSubInterpreters;
    }

    QPyThreadPool& QPyThreadPool::instance() {
        static std::unique_ptr<QPyThreadPool> singleton;
        static std::mutex m;
        std::lock_guard<std::mutex> lock(m);
        if (!_initialized) {
            if (_useSubInterpreters) {
                Py_Initialize();
                saveMainThreadState();
            }
            Py_Initialize();

            singleton = std::unique_ptr<QPyThreadPool>(new QPyThreadPool(_threadCount, _useSubInterpreters));
            _initialized = true;
        }
        return *singleton;
    }

    void QPyThreadPool::saveMainThreadState() {
        _mainThreadState = PyEval_SaveThread();
    }

    void QPyThreadPool::restoreMainThreadState() {
        PyEval_RestoreThread(_mainThreadState);
    }

    QPyThreadPool::~QPyThreadPool() {
        shutdown();
    }

    void QPyThreadPool::submit(QPyFuture future) {
        auto key = future.callablePtr();
        static std::atomic<size_t> rr{0};
        size_t idx = rr.fetch_add(1) % tasks_.size();
        if (m_threadCallableMap_.contains(key)) {
            idx = m_threadCallableMap_[key];
        } else {
            m_threadCallableMap_[key] = static_cast<int>(idx);
        }
        tasks_[idx]->push(std::move(future));
    }

    void QPyThreadPool::shutdown() {
        if (bool expected = false; !stop_.compare_exchange_strong(expected, true))
            return;

        // close all queues so workers unblock
        for (auto& q : tasks_)
            q->close();

        for (auto& t : workers_) {
            if (t.joinable())
                t.join();
        }
        workers_.clear();

        if (_useSubInterpreters) {
            restoreMainThreadState();
            PyGILState_STATE gstate = PyGILState_Ensure();
            Py_Finalize();
            PyGILState_Release(gstate);
            restoreMainThreadState();
        }
    }
} // namespace qtpyt