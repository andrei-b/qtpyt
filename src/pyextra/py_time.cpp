//
// Created by andrei on 06.12.25.
//

#include "py_time.h"

namespace py_extra {

py_time::py_time(const QTime& t) {
    const pybind11::module_ datetime = pybind11::module_::import("datetime");
    obj = datetime.attr("time")(t.hour(), t.minute(), t.second());
}

QTime py_time::to_qtime() const {
    return {obj.attr("hour").cast<int>(), obj.attr("minute").cast<int>(), obj.attr("second").cast<int>(),
            obj.attr("microsecond").cast<int>() / 1000};
}

py_time::py_time(int hour, int minute, int second, int microsecond) {
    const pybind11::module_ datetime = pybind11::module_::import("datetime");
    obj = datetime.attr("time")(hour, minute, second, microsecond);
}

bool py_time::is_instance(const pybind11::object& o) {
    const pybind11::module_ datetime = pybind11::module_::import("datetime");
    return pybind11::isinstance(o, datetime.attr("time"));
}

py_time make_py_time(int hour, int minute, int second, int microsecond) {
    return { hour, minute, second, microsecond };
}

} // namespace py_extra