#include <qtpyt/pycall.h>

namespace pycall_internal__ {

    py::object build_args_tuple_from_variant_list(const QVariantList &args) {
        const int n = static_cast<int>(args.size());
        py::tuple tup(n);
        for (int i = 0; i < n; ++i) {
            tup[i] = qtpyt::qvariantToPyObject(args[i]);
        }
        return py::reinterpret_borrow<py::object>(tup.ptr());
    }

    pybind11::object call_python(const py::object& callable, const pybind11::tuple& args, const pybind11::dict& kwargs) {
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
                throw pybind11::error_already_set();
            }
            return py::reinterpret_steal<py::object>(res_ptr);
        } catch (const py::error_already_set &e) {
            throw std::runtime_error(std::string("Python error: ") + e.what());
        }
    }

    pybind11::object call_python_no_kw(const py::object& callable, const pybind11::tuple& args) {
        return call_python(callable, args, py::dict());
    }
} // namespace pycall_internal__