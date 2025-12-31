#include <qtpyt/qpymodulebase.h>
#include "q_embed_meta_object_py.h"
#include "internal/annotations.h"
#include <qtpyt/conversions.h>

namespace qtpyt {
    QPyModuleBase::QPyModuleBase(const QString &source, const QPySourceType sourceType,
                                 const QString &funcName) : m_callableFunction(funcName) {
        switch (sourceType) {
            case QPySourceType::File:
                buildFromFile(source, funcName);
                break;
            case QPySourceType::SourceString:
                buildFromString(source, funcName);
                break;
            case QPySourceType::Module:
                break;
        }
    }

    QPyModuleBase::~QPyModuleBase() {

            // Destructors can run on arbitrary threads and/or during interpreter finalization.
            // Avoid refcount operations unless Python is initialized and this thread has a valid state.
            auto safe_release = [](pybind11::object &o) {
                if (!o || o.is_none()) return;

                if (!Py_IsInitialized() || PyGILState_GetThisThreadState() == nullptr) {
                    // Do not decref: can trip PyGILState_Check()/invalid tstate.
                    o.release();
                    return;
                }

                pybind11::gil_scoped_acquire gil;
                // Ensure decref happens while GIL is held.
                o = pybind11::none();
            };

            safe_release(callable);
            safe_release(m_module);

            m_isValid = false;

    }

    bool QPyModuleBase::isValid() const {
        return m_isValid;
    }

    std::optional<QVariant> QPyModuleBase::call(const QString &function, const QPyRegisteredType& returnType,
                                                const QVariantList &args, const QVariantMap &kwargs) {
        try {
            QByteArray typeName = {"Unknown Type"};
            if (std::holds_alternative<QMetaType>(returnType)) {
                typeName = std::get<QMetaType>(returnType).name();
            } else if (std::holds_alternative<QMetaType::Type>(returnType)) {
                typeName = QMetaType(std::get<QMetaType::Type>(returnType)).name();
            } else if (std::holds_alternative<QString>(returnType)) {
                typeName = QByteArray::fromStdString(std::get<QString>(returnType).toStdString());
            }
            const auto argsTuple = pycall_internal__::build_args_tuple_from_variant_list(args);
            py::dict kwargsDict;
            for (auto it = kwargs.constBegin(); it != kwargs.constEnd(); ++it) {
                kwargsDict[py::str(it.key().toStdString())] = qvariantToPyObject(it.value());
            }
            setCallableFunction(function);
            return pyObjectToQVariant(pycall_internal__::call_python(callable, argsTuple, kwargsDict),
                typeName);
        } catch (const std::exception &e) {
            return std::nullopt;
        }
        return {};
    }

    std::optional<py::object> QPyModuleBase::makeCallable(const QString &function) {
        try {
            return m_module.attr(function.toStdString().c_str());
        } catch (const std::exception &e) {
            qWarning() << "QPyModuleBase::makeCallable error:" << e.what();
            return std::nullopt;
        }
    }

    void QPyModuleBase::setCallableFunction(const QString &name) {
        m_callableFunction = name;
        if (m_module && !m_module.is_none()) {
            py::gil_scoped_acquire gil;
            py::object func = m_module.attr(name.toStdString().c_str());
            if (!func.is_none() && PyCallable_Check(func.ptr())) {
                callable = func;
                m_isValid = true;
            } else {
                callable = py::none();
                m_isValid = false;
            }
        }
    }

    pybind11::object QPyModuleBase::call(const pybind11::tuple &args, const pybind11::dict &kwargs) const {
        try {
            const py::object result = pycall_internal__::call_python(callable, args, kwargs);
            return result;
        } catch (const std::exception &e) {
            return py::none();
        }
        return py::none();
    }

    pybind11::object &&QPyModuleBase::takePythonCallable(const QString &functionName) {
        py::gil_scoped_acquire gil;
        py::object func;
        if (functionName.isEmpty()) {
            func = callable;
            callable = py::none();
        } else {
            func = m_module.attr(functionName.toStdString().c_str());
            if (!func.is_none() && PyCallable_Check(func.ptr())) {
                callable = py::none();
            } else {
                func = py::none();
            }
        }
        return std::move(func);
    }

    PyCallableInfo QPyModuleBase::inspectCallable() const {
        py::gil_scoped_acquire gil;
        PyCallableInfo info;

        if (!callable || callable.is_none()) {
            return info;
        }

        py::module_ inspect = py::module_::import("inspect");
        auto getArgType = [](const pybind11::handle &h) -> QPyValueType {
            if (!h || h.is_none()) return QPyValueType::Any;
            using namespace pybind11;
            if (isinstance<int_>(h)) return QPyValueType::Int;
            if (isinstance<float_>(h)) return QPyValueType::Float;
            if (isinstance<bool_>(h)) return QPyValueType::Bool;
            if (isinstance<str>(h)) return QPyValueType::Str;
            if (isinstance<list>(h)) return QPyValueType::List;
            if (isinstance<tuple>(h)) return QPyValueType::Tuple;
            if (isinstance<dict>(h)) return QPyValueType::Dict;
            if (isinstance<function>(h)) return QPyValueType::Callable;
            return QPyValueType::Any;
        };
        try {
            // Prefer getfullargspec for a simple structured view
            py::object spec = inspect.attr("getfullargspec")(callable);
            for (py::list args_py = spec.attr("args"); auto &it: args_py) {
                const auto arg_name = it.cast<std::string>();
                py::object ann_obj = spec.attr("annotations").attr("get")(arg_name, py::none());
                QPyValueType argType = getArgType(ann_obj);
                info.arguments.push_back({QPyArgumentType::Argument, argType, QString::fromStdString(arg_name)});
            }

            // varargs / varkw
            if (py::object varargs = spec.attr("varargs"); !varargs.is_none()) {
                info.has_varargs = true;
                info.arguments.push_back({
                    QPyArgumentType::VarArgs, QPyValueType::Any, QString::fromStdString(varargs.cast<std::string>())
                });
            }
            if (py::object varkw = spec.attr("varkw"); !varkw.is_none()) {
                info.has_varkw = true;
                info.arguments.push_back({
                    QPyArgumentType::VarKwargs, QPyValueType::Any, QString::fromStdString(varkw.cast<std::string>())
                });
            }

            // defaults: tuple aligned to last N args
            py::object defaults = spec.attr("defaults");
            if (!defaults.is_none()) {
                auto def_tup = defaults.cast<py::tuple>();
                int ndefs = (int) def_tup.size();
                int nargs = (int) info.arguments.size();
                for (int i = nargs - ndefs; i < nargs; ++i) {
                    info.arguments[i].hasDefault = true;
                }
            }
            // annotations
            py::dict ann = spec.attr("annotations");
            for (auto item: ann) {
                std::string name = item.first.cast<std::string>();
                info.annotations[name] = item.second;
            }
        } catch (const py::error_already_set &e) {
            // Fallback: try inspect.signature when getfullargspec fails (e.g., builtins/C functions)
            py::object sig = inspect.attr("signature")(callable);
            py::object params = sig.attr("parameters");
            py::list items = params.attr("items")();
            for (auto &it: items) {
                auto pair = it.cast<py::tuple>();
                auto name = pair[0].cast<std::string>();
                py::object param = pair[1];
                auto type = getArgType(param.attr("annotation"));
                bool hasDefault = false;
                auto argType = QPyArgumentType::Argument;
                std::string kind = py::str(param.attr("kind"));
                if (kind.find("VAR_POSITIONAL") != std::string::npos) {
                    info.has_varargs = true;
                    argType = QPyArgumentType::VarArgs;
                } else if (kind.find("VAR_KEYWORD") != std::string::npos) {
                    info.has_varkw = true;
                    argType = QPyArgumentType::VarKwargs;
                }
                if (py::object default_val = param.attr("default");
                    !default_val.is_none() && default_val.ptr() != inspect.attr("_empty").ptr()) {
                    hasDefault = true;
                }

                QPyAnnotation annotation;

                if (py::handle ann = param.attr("annotation");
                    !ann.is_none() && ann.ptr() != inspect.attr("_empty").ptr()) {
                    annotation = annotations_internal__::parse_annotation(ann, 0);
                }

                info.arguments.push_back({argType, type, QString::fromStdString(kind), hasDefault, annotation});
            }
        }
        return info;
    }

    QString QPyModuleBase::functionName() const {
        return m_callableFunction;
    }

    void QPyModuleBase::addVariable(const QString &name, const QVariant &value) {
        py::gil_scoped_acquire gil;
        if (m_module && !m_module.is_none()) {
            m_module.attr(name.toStdString().c_str()) = qvariantToPyObject(value);
        }
    }

    QVariant QPyModuleBase::readVariable(const QString &name, const QPyRegisteredType &type) const {
        py::gil_scoped_acquire gil;
        if (m_module && !m_module.is_none()) {
            py::object var = m_module.attr(name.toStdString().c_str());
            QByteArray typeName = {"Unknown Type"};
            if (std::holds_alternative<QMetaType>(type)) {
                typeName = std::get<QMetaType>(type).name();
            } else if (std::holds_alternative<QMetaType::Type>(type)) {
                typeName = QMetaType(std::get<QMetaType::Type>(type)).name();
            } else if (std::holds_alternative<QString>(type)) {
                typeName = QByteArray::fromStdString(std::get<QString>(type).toStdString());
            }
            if (auto result = qtpyt::pyObjectToQVariant(var, typeName); result.has_value()) {
                return result.value();
            }
        }
        return {};
    }

    py::object &QPyModuleBase::getPyCallable() {
        return callable;
    }

    void QPyModuleBase::buildFromString(const QString &source, const QString &funcName) {
        try {
            py::gil_scoped_acquire acquire;

            const std::string mod_name = std::string("_pycall_src_") + std::to_string(
                                             std::hash<std::string>{}(source.toStdString()));

            py::module_ types = py::module_::import("types");
            m_module = types.attr("ModuleType")(mod_name);

            py::dict globals = m_module.attr("__dict__").cast<py::dict>();
            py::module_ builtins = py::module_::import("builtins");
            globals["__builtins__"] = builtins.attr("__dict__"); // ensures `print` and other builtins are available

            py::exec(source.toStdString(), globals);

            callable = m_module.attr(funcName.toStdString().c_str());
            if (!callable || !PyCallable_Check(callable.ptr())) {
                m_isValid = false;
            }
            m_isValid = true;
        } catch (const py::error_already_set &e) {
            throw std::runtime_error(std::string("Python error: ") + e.what());
        }
    }

    void QPyModuleBase::buildFromFile(const QString &fileName, const QString &funcName) {
        try {
            py::gil_scoped_acquire acquire;

            const std::string spec_name = std::string("_pycall_") + std::to_string(
                                              std::hash<std::string>{}(fileName.toStdString()));

            // importlib.util.spec_from_file_location + module_from_spec + loader.exec_module(module)
            const py::module_ importlib_util = py::module_::import("importlib.util");
            py::object spec = importlib_util.attr("spec_from_file_location")(
                spec_name.c_str(), fileName.toStdString().c_str());
            if (!spec || spec.is_none()) throw std::runtime_error("failed to create spec for module path");
            m_module = importlib_util.attr("module_from_spec")(spec);
            if (!m_module || m_module.is_none()) throw std::runtime_error("failed to create module from spec");

            py::object loader = spec.attr("loader");
            if (!loader || loader.is_none()) throw std::runtime_error("spec.loader is null for module path");
            loader.attr("exec_module")(m_module);

            callable = m_module.attr(funcName.toStdString().c_str());
            if (!callable || !PyCallable_Check(callable.ptr())) {
                m_isValid = false;
            }
            m_isValid = true;
        } catch (const py::error_already_set &e) {
            throw std::runtime_error(std::string("Python error: ") + e.what());
        }
    }
} // namespace qtpyt
