// cpp
#pragma once
#include <Python.h>
#include <pybind11/pybind11.h>
#include <utility>
#include <functional>

class SubInterpreter {
public:
    SubInterpreter();
  template <typename F>
    void run(F&& fn) {
        PyEval_RestoreThread(saved_tstate_); // acquires GIL and sets threadstate
        try {
            std::forward<F>(fn)(); // safe to use pybind11 here
        } catch (...) {
            saved_tstate_ = PyEval_SaveThread();
            throw;
        }
        saved_tstate_ = PyEval_SaveThread();
    }

    ~SubInterpreter();

    // non-copyable
    SubInterpreter(const SubInterpreter&) = delete;
    SubInterpreter& operator=(const SubInterpreter&) = delete;

    // move allowed
    SubInterpreter(SubInterpreter&& o) noexcept;
    SubInterpreter& operator=(SubInterpreter&&) = delete;

private:
    PyGILState_STATE gil_state_;
    PyThreadState* tstate_{nullptr};      // interpreter's thread state (returned by Py_NewInterpreter)
    PyThreadState* saved_tstate_{nullptr}; // thread state saved by PyEval_SaveThread
};
