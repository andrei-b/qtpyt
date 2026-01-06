
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include "pymodule.h"
#include "q_embed_meta_object_py.h"

static_assert(PYBIND11_VERSION_HEX >= 0x020D0500, "Wrong/old pybind11 headers");

namespace qtpyt {
    namespace py = pybind11;

    namespace {
        static py::tuple invoke_from_variant_list_wrapper(uintptr_t obj_ptr, const std::string& method, const py::args& args) {
            py::object return_value = py::none();
            bool ok = invoke_from_args(obj_ptr, method, return_value, args);
            return py::make_tuple(ok, return_value);
        }

        extern "C" {
            // CPython will call this to create the module
            PyObject* PyInit_qembed();
        }


        PYBIND11_EMBEDDED_MODULE(qembed, m, py::mod_gil_not_used()) {
            m.doc() = "pybind11 bindings for QEmbedMetaObject invokeFromVariantListDynamic";

            // wrapper: (uintptr_t obj_ptr, const std::string& method, py::args args)
            m.def("invoke_from_variant_list", &invoke_from_variant_list, py::arg("obj_ptr"),
                  py::arg("method"), py::arg("return_value"), py::arg("args"));

            m.def("invoke", &invoke_returning_from_args, py::arg("obj_ptr"), py::arg("method"));

            m.def("invoke_ret_void", &invoke_void_from_args, py::arg("obj_ptr"), py::arg("method"));


            // find_object_by_name(root_ptr, name, recursive=true)
            m.def("find_object_by_name", &find_object_by_name, py::arg("root_ptr"), py::arg("name"),
                  py::arg("recursive") = true);

            // set_property(obj_ptr, property_name, value)
            m.def("set_property", &set_property, py::arg("obj_ptr"), py::arg("property_name"),
                  py::arg("value"));
            m.def("set_property_async", &set_property_async, py::arg("obj_ptr"), py::arg("property_name"),
                             py::arg("value"));
            // get_property(obj_ptr, property_name)
            m.def("get_property", &get_property, py::arg("obj_ptr"), py::arg("property_name"));

            m.def("get_property_mt", &get_property_mt, py::arg("obj_ptr"), py::arg("property_name"));

            m.def("invoke_mt", &invoke_returning_from_args_mt, py::arg("obj_ptr"), py::arg("method"));

        }

    } // namespace


    static bool initialized{false};

    static std::unique_ptr<py::scoped_interpreter> global_interpreter;

    static std::mutex m;

    static const auto module_name = "qembed";

    void makeEmbeddedModule(QObject * root_obj) {
        std::lock_guard<std::mutex> lock(m);
        if (!initialized) {
            initialized = true;

            if (!Py_IsInitialized()) {
                global_interpreter = std::make_unique<py::scoped_interpreter>();
            }

            pybind11::gil_scoped_acquire acquire;

            py::module_ qembed = py::module_::import(module_name);
            const auto ptr = reinterpret_cast<uintptr_t>(root_obj);
            py::int_ py_ptr = py::int_(ptr);
            qembed.attr("root_obj") = py_ptr;
        }
    }

} // namespace qtpyt