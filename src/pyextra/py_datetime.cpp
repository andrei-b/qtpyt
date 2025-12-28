#include "py_datetime.h"


namespace py_extra {

py_datetime::py_datetime(const QDateTime& dt) {
    const pybind11::module_ datetime = pybind11::module_::import("datetime");
    QDate d = dt.date();
    QTime t = dt.time();
    int micro = t.msec() * 1000 + (t.isValid() ? 0 : 0); // Qt only exposes msec; keep as microseconds approximated
    obj = datetime.attr("datetime")(
        d.year(), d.month(), d.day(),
        t.hour(), t.minute(), t.second(), micro);
}

py_datetime::py_datetime(int year, int month, int day, int hour, int minute, int second, int microsecond) {
    const pybind11::module_ datetime = pybind11::module_::import("datetime");
    obj = datetime.attr("datetime")(year, month, day, hour, minute, second, microsecond);
}

QDateTime py_datetime::to_qdatetime() const {
    int year = obj.attr("year").cast<int>();
    int month = obj.attr("month").cast<int>();
    int day = obj.attr("day").cast<int>();
    int hour = obj.attr("hour").cast<int>();
    int minute = obj.attr("minute").cast<int>();
    int second = obj.attr("second").cast<int>();
    int micro = obj.attr("microsecond").cast<int>();
    QDate date(year, month, day);
    QTime time(hour, minute, second, micro / 1000); // convert micro -> msec
    QDateTime dt(date, time);
    pybind11::object tzinfo = obj.attr("tzinfo");
    if (!tzinfo.is_none()) {
        pybind11::object utcoffset = tzinfo.attr("utcoffset")(obj);
        if (!utcoffset.is_none()) {
            int total_seconds = utcoffset.attr("total_seconds")().cast<int>();
            dt.setOffsetFromUtc(total_seconds / 60); // offset in minutes
        }
    }
    return dt;
}

pybind11::object qdatetime_to_pyobject(const QDateTime &dt)
{
    namespace py = pybind11;
    py::module_ datetime = py::module_::import("datetime");

    py::object py_datetime_class = datetime.attr("datetime");
    py::object py_dt = py_datetime_class(
        dt.date().year(),
        dt.date().month(),
        dt.date().day(),
        dt.time().hour(),
        dt.time().minute(),
        dt.time().second(),
        dt.time().msec() * 1000 // microseconds
    );

    if (dt.timeSpec() == Qt::UTC) {
        py::module_ pytz = py::module_::import("pytz");
        py::object utc = pytz.attr("utc");
        py_dt = py_dt.attr("replace")(py::arg("tzinfo") = utc);
    } else if (dt.timeSpec() == Qt::LocalTime) {
        py::module_ tzlocal = py::module_::import("tzlocal");
        py::object local_tz = tzlocal.attr("get_localzone")();
        py_dt = py_dt.attr("replace")(py::arg("tzinfo") = local_tz);
    }

    return py_dt;
}

bool py_datetime::is_instance(const pybind11::object& o) {
    try {
        pybind11::module_ datetime = pybind11::module_::import("datetime");
        return pybind11::isinstance(o, datetime.attr("datetime"));
    } catch (const pybind11::error_already_set &) {
        return false;
    }
}

py_datetime make_py_datetime(int year, int month, int day, int hour, int minute, int second, int microsecond) {
    return { year, month, day, hour, minute, second, microsecond };
}


} // namespace py_extra
