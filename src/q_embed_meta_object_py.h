#pragma once
#define PYBIND11_NO_KEYWORDS
#include <pybind11/embed.h>

namespace py = pybind11;

namespace qtpyt {
        uintptr_t find_object_by_name(const uintptr_t root_ptr, const std::string& name, const bool recursive = true);

        bool invoke_from_args(const uintptr_t obj_ptr, const std::string& method, py::object& retValue, const py::args& args);

        inline void invoke_void_from_args(const uintptr_t obj_ptr, const std::string& method, const py::args& args) {
            py::object retValue; // unused
            invoke_from_args(obj_ptr, method, retValue, args);
        }

        py::object invoke_returning_from_args(const uintptr_t obj_ptr, const std::string& method, const py::args& args);

        bool invoke_from_variant_list(uintptr_t obj_ptr, const std::string& method, py::object& return_value,
                                             const py::iterable& args);

        bool set_property(uintptr_t obj_ptr, const std::string& property_name, const py::object& value);

        bool set_property_async(uintptr_t obj_ptr, const std::string& property_name, const py::object& value);

        py::object get_property(uintptr_t obj_ptr, const std::string& property_name);

        py::object get_property_mt(uintptr_t obj_ptr, const std::string& property_name);

        py::object invoke_returning_from_args_mt(const uintptr_t obj_ptr, const std::string &method,
            const py::args &args);

} // namespace qtpyt