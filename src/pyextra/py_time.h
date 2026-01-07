//
// Created by andrei on 06.12.25.
//

#pragma once

#define PYBIND11_NO_KEYWORDS
#include <pybind11/pybind11.h>
#include <QTime>

namespace py_extra {

class py_time {
    pybind11::object obj;
public:
    py_time() = default;
    explicit py_time(pybind11::object o) : obj(std::move(o)) {}
    explicit py_time(const QTime &t);
    explicit operator pybind11::object() const { return obj; }
    QTime to_qtime() const;
    static bool is_instance(const pybind11::object &o);
    py_time(int hour = 0, int minute = 0, int second = 0, int microsecond = 0);
    py_time(const py_time&) = default;
    py_time& operator=(const py_time&) = default;
};

py_time make_py_time(int hour = 0, int minute = 0, int second = 0, int microsecond = 0);

} // namespace py_extra

