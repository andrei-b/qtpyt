#pragma once

#include <qtpyt/conversions.h>
#ifdef slots
  #pragma push_macro("slots")
  #undef slots
  #define _PY_SLOTS_WAS_UNDEF
#endif
#include <pybind11/pybind11.h>
#ifdef _PY_SLOTS_WAS_UNDEF
  #pragma pop_macro("slots")
  #undef _PY_SLOTS_WAS_UNDEF
#endif
#include <optional>
#include <QString>
#include <QVariant>

namespace pycall_internal__ {
template<typename... Args>
py::object build_args_tuple(Args&&... args) {
    constexpr size_t N = sizeof...(Args);
    py::tuple tup(static_cast<py::ssize_t>(N));
    int idx = 0;
    (void)std::initializer_list<int>{
        (tup[static_cast<py::ssize_t>(idx++)] =
             qvariantToPyObject(std::forward<Args>(args)), 0)...
    };
    return py::reinterpret_borrow<py::object>(tup.ptr());
}

inline py::object build_args_tuple_from_variant_list(const QVariantList& args) {
    const int n = static_cast<int>(args.size());
    py::tuple tup(n);
    for (int i = 0; i < n; ++i) {
        tup[i] = qtpyt::qvariantToPyObject(args[i]);
    }
    return py::reinterpret_borrow<py::object>(tup.ptr());
}


inline pybind11::object call_python(const py::object& callable, const pybind11::tuple& args, const pybind11::dict& kwargs = {}) {
    if (!callable) throw std::runtime_error("callable is null");
    if (!PyCallable_Check(callable.ptr())) {
        throw std::runtime_error("object is not callable");
    }
    if (!args) {
        throw std::runtime_error("args is null");
    }
    try {
        //py::gil_scoped_acquire acquire;

        PyObject *res_ptr = PyObject_Call(callable.ptr(), args.ptr(), kwargs.ptr());
        if (!res_ptr) {
            PyErr_Print();
            throw std::runtime_error("Python call failed");
        }
        return py::reinterpret_steal<py::object>(res_ptr);
    } catch (const py::error_already_set &e) {
        throw std::runtime_error(std::string("Python error: ") + e.what());
    }
}

template<typename R, typename... Args>
R call_python(py::object callable, Args&&... args) {

    if (!callable) throw std::runtime_error("callable is null");
    try {
        py::object arg_tuple_obj = pycall_internal__::build_args_tuple(std::forward<Args>(args)...);
        auto res = call_python(callable, arg_tuple_obj);
        const auto result = py::reinterpret_steal<py::object>(res.ptr());

        if constexpr (std::is_same_v<R, void>) {
            return;
        } else {
            auto typeName = QMetaType::typeName(qMetaTypeId<R>());
            if (auto a = pyObjectToQVariant(result, typeName); a.has_value()) {
                auto result = a.value().template value<R>();
                return result;
            }
            qWarning("pycall_internal__::call_python: Failed to convert Python return value to expected C++ type");
            return {};
        }
    } catch (const py::error_already_set &e) {
        throw std::runtime_error(std::string("Python error: ") + e.what());
    }
}




}