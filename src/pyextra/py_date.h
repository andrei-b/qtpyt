//
// Created by andrei on 06.12.25.
//

#pragma once
#define PYBIND11_NO_KEYWORDS
#include <pybind11/pybind11.h>
#include <QDate>
#include <QTime>
#include <QDateTime>

namespace py_extra {


class py_date {
    pybind11::object obj;
public:
    py_date() = default;
    explicit py_date(pybind11::object o) : obj(std::move(o)) {}
    explicit py_date(const QDate &d);
    explicit operator pybind11::object() const { return obj; }
    QDate to_qdate() const;
    static bool is_instance(const pybind11::object &o);
    py_date(int year, int month, int day);
};

py_date make_py_date(int year, int month, int day);


} // namespace py_extra

