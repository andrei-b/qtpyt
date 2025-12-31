#define PYBIND11_NO_KEYWORDS

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
#include <qtpyt/qpyscript.h>
#include  "pymodule.h"

namespace qtpyt {

namespace py = pybind11;

static const char* names[] = {"get_property", "set_property", "invoke", "find_object_by_name",
                                    "invoke_from_variant_list", "invoke_ret_void"};





std::tuple<bool, QString> QPyScript::runScriptFileGlobal(const QString& script_path, QObject* root_obj) {
    try {
        makeEmbeddedModule(root_obj);
        py::module_ sys = py::module_::import("sys");

        py::module_ main = py::module_::import("__main__");
        const auto ptr = reinterpret_cast<uintptr_t>(root_obj);
        py::int_ py_ptr = py::int_(ptr);
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
        makeEmbeddedModule(root_obj);
        py::module_ main = py::module_::import("__main__");
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