#pragma once
#define PYBIND11_NO_KEYWORDS

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "q_embed_meta_object.h" // existing header in project

#include <pybind11/embed.h>

namespace py = pybind11;

namespace qtpyt {
    class QEmbedMetaObjectPy
    {
    public:
        static uintptr_t find_object_by_name(const uintptr_t root_ptr, const std::string& name, const bool recursive = true);

        static bool invoke_from_args(const uintptr_t obj_ptr, const std::string& method, py::object& retValue, const py::args& args);

        static void invoke_void_from_args(const uintptr_t obj_ptr, const std::string& method, const py::args& args) {
            py::object retValue; // unused
            QEmbedMetaObjectPy::invoke_from_args(obj_ptr, method, retValue, args);
        }

        static py::object invoke_returning_from_args(const uintptr_t obj_ptr, const std::string& method, const py::args& args) {
            py::object retValue;
            QEmbedMetaObjectPy::invoke_from_args(obj_ptr, method, retValue, args);
            return retValue;
        }

        // obj_ptr: numeric pointer to QObject (uintptr_t)
        // method: method name (e.g. "setWindowTitle")
        // args: Python list of arguments convertible to QVariant
        static bool invoke_from_variant_list(uintptr_t obj_ptr, const std::string& method, py::object& return_value,
                                             const py::iterable& args);

        static bool set_property(uintptr_t obj_ptr, const std::string& property_name, const py::object& value);
        static py::object get_property(uintptr_t obj_ptr, const std::string& property_name);
    };
} // namespace qtpyt