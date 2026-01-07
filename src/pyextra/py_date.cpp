//
// Created by andrei on 06.12.25.
//

#include "py_date.h"

namespace py_extra {

py_date::py_date(int year, int month, int day) {
    const pybind11::module_ datetime = pybind11::module_::import("datetime");
    obj = datetime.attr("date")(year, month, day);
}

py_date::py_date(const QDate& d) {
    const pybind11::module_ datetime = pybind11::module_::import("datetime");
    obj = datetime.attr("date")(d.year(), d.month(), d.day());
}

QDate py_date::to_qdate() const {
    return {obj.attr("year").cast<int>(), obj.attr("month").cast<int>(), obj.attr("day").cast<int>()};
}

bool py_date::is_instance(const pybind11::object &o) {
    try {
        pybind11::module_ datetime = pybind11::module_::import("datetime");
        return pybind11::isinstance(o, datetime.attr("date"));
    } catch (const pybind11::error_already_set &) {
        return false;
    }
}

py_date make_py_date(int year, int month, int day) {
    return py_date(year, month, day);
}


} // namespace py_extra