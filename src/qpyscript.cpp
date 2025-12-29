#define PYBIND11_NO_KEYWORDS

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
#include <qtpyt/qpyscript.h>

namespace qtpyt {

namespace py = pybind11;

static const char* names[] = {"get_property", "set_property", "invoke", "find_object_by_name",
                                    "invoke_from_variant_list", "invoke_ret_void"};

static std::unique_ptr<py::scoped_interpreter> global_interpreter;



std::tuple<bool, QString> QPyScript::runScriptFileGlobal(const QString& script_path, QObject* root_obj) {
    try {
        if (!Py_IsInitialized()) {
            global_interpreter = std::make_unique<py::scoped_interpreter>();
        }

       pybind11::gil_scoped_acquire acquire;
        py::module_ sys = py::module_::import("sys");
        sys.attr("path").attr("insert")(0, "/home/andrei/qttest/cmake-build-debug/qembed.py");
       py::module_ qembed = py::module_::import("qembed");

        py::module_ main = py::module_::import("__main__");
        const auto ptr = reinterpret_cast<uintptr_t>(root_obj);
        py::int_ py_ptr = py::int_(ptr);
        qembed.attr("root_obj") = py_ptr;

        for (const char* name : names) {
            if (py::hasattr(qembed, name)) {
                //main.attr(name) = qembed.attr(name);
            }
        }

        py::module_ runpy = py::module_::import("runpy");
        runpy.attr("run_path")(script_path.toStdString());

        return {true, {}};
    } catch (const py::error_already_set& e) {
        return {false, QStringLiteral("Python error: ").append(QString::fromStdString(e.what()))};
    } catch (const std::exception& e) {
        return {false, QString::fromStdString(e.what())};
    };
}

std::tuple<bool, QString> QPyScript::runScriptGlobal(const QString& script, QObject* root_obj) {
    try {
        if (!Py_IsInitialized()) {
            global_interpreter = std::make_unique<py::scoped_interpreter>();
        }

        //py::gil_scoped_acquire acquire;

        py::module_ qembed = py::module_::import("qembed");
        py::module_ main = py::module_::import("__main__");

        const auto ptr = reinterpret_cast<uintptr_t>(root_obj);
        py::int_ py_ptr = py::int_(ptr);
        qembed.attr("root_obj") = py_ptr;

        py::dict globals = main.attr("__dict__");
        py::exec(script.toStdString(), globals);

        return {true, {}};
    } catch (const py::error_already_set& e) {
        return {false, QStringLiteral("Python error: ").append(QString::fromStdString(e.what()))};
    } catch (const std::exception& e) {
        return {false, QString::fromStdString(e.what())};
    }
}



} // namespace qtpyt