#pragma once

#define PYBIND11_NO_KEYWORDS
#include <pybind11/pybind11.h>
#include <QDateTime>

namespace py_extra {

class py_datetime {
    pybind11::object obj;
  public:
    py_datetime() = default;
    explicit py_datetime(pybind11::object o) : obj(std::move(o)) {}
    explicit py_datetime(const QDateTime& dt);
    explicit operator pybind11::object() const {
        return obj;
    }
    QDateTime to_qdatetime() const;
    static bool is_instance(const pybind11::object& o);
    py_datetime(int year, int month, int day, int hour = 0, int minute = 0, int second = 0, int microsecond = 0);
};

py_datetime make_py_datetime(int year, int month, int day, int hour = 0, int minute = 0, int second = 0, int microsecond = 0);

} // namespace py_extra
