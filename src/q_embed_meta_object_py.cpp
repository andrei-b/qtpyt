#define PYBIND11_DETAILED_ERROR_MESSAGES
#define PYBIND11_NO_KEYWORDS
#include <pybind11/embed.h>
#include <pybind11/detail/common.h>
#include "q_embed_meta_object_py.h"

#include <qtpyt/conversions.h>
#include "q_embed_meta_object.h"
#include <QColor>
#include <QJsonArray>
#include <QJsonObject>
#include <QMetaMethod>
#include <QVariantList>
#include <qpoint.h>



#pragma message("PYBIND11 file: " PYBIND11_STRINGIFY(PYBIND11_INTERNALS_ID))

static_assert(PYBIND11_VERSION_HEX >= 0x020D0500, "Wrong/old pybind11 headers");

namespace qtpyt {
    namespace {
        static py::tuple invoke_from_variant_list_wrapper(uintptr_t obj_ptr, const std::string& method, const py::args& args) {
            py::object return_value = py::none();
            bool ok = QEmbedMetaObjectPy::invoke_from_args(obj_ptr, method, return_value, args);
            return py::make_tuple(ok, return_value);
        }

        extern "C" {
            // CPython will call this to create the module
            PyObject* PyInit_qembed();
        }

#if PY_VERSION_HEX >= 0x030D0000
        // Declare module can run with GIL disabled (free-threaded)
        static PyModuleDef_Slot qembed_slots[] = {
            {Py_mod_gil, Py_MOD_GIL_NOT_USED},
            {0, nullptr}
        };
#endif

        PYBIND11_EMBEDDED_MODULE(qembed, m) {
            m.doc() = "pybind11 bindings for QEmbedMetaObject invokeFromVariantListDynamic";

            // wrapper: (uintptr_t obj_ptr, const std::string& method, py::args args)
            m.def("invoke_from_variant_list", &QEmbedMetaObjectPy::invoke_from_variant_list, py::arg("obj_ptr"),
                  py::arg("method"), py::arg("return_value"), py::arg("args"));

            m.def("invoke", &QEmbedMetaObjectPy::invoke_returning_from_args, py::arg("obj_ptr"), py::arg("method"));

            m.def("invoke_ret_void", &QEmbedMetaObjectPy::invoke_void_from_args, py::arg("obj_ptr"), py::arg("method"));


            // find_object_by_name(root_ptr, name, recursive=true)
            m.def("find_object_by_name", &QEmbedMetaObjectPy::find_object_by_name, py::arg("root_ptr"), py::arg("name"),
                  py::arg("recursive") = true);

            // set_property(obj_ptr, property_name, value)
            m.def("set_property", &QEmbedMetaObjectPy::set_property, py::arg("obj_ptr"), py::arg("property_name"),
                  py::arg("value"));

            // get_property(obj_ptr, property_name)
            m.def("get_property", &QEmbedMetaObjectPy::get_property, py::arg("obj_ptr"), py::arg("property_name"));


#if PY_VERSION_HEX >= 0x030D0000
            // Patch the module definition to include the slot.
            // This uses pybind11 internals (stable enough across 2.13.x).
            auto *def = reinterpret_cast<PyModuleDef*>(PyModule_GetDef(m.ptr()));
            def->m_slots = qembed_slots;
#endif

        }

    } // namespace


    uintptr_t QEmbedMetaObjectPy::find_object_by_name(const uintptr_t root_ptr, const std::string& name,
                                                      const bool recursive) {
        const QObject* root = reinterpret_cast<QObject*>(static_cast<uintptr_t>(root_ptr));
        if (!root) {
            return 0;
        }

        const QString qname = QString::fromStdString(name);

        if (recursive) {
            auto found = root->findChild<QObject*>(qname);
            return reinterpret_cast<uintptr_t>(found);
        } else {
            for (const auto children = root->children(); QObject * c : children) {
                if (c && c->objectName() == qname) {
                    return reinterpret_cast<uintptr_t>(c);
                }
            }
            return 0;
        }
    }
    bool QEmbedMetaObjectPy::invoke_from_args(const uintptr_t obj_ptr, const std::string& method, py::object& retValue,
                                              const py::args& args) {
        // forward to existing implementation which accepts a py::iterable
        return QEmbedMetaObjectPy::invoke_from_variant_list(obj_ptr, method, retValue,
                                                            py::reinterpret_borrow<py::iterable>(args));
    }


    void invoke_void_from_args(const uintptr_t obj_ptr, const std::string& method, const py::args& args) {
        py::object retValue; // unused
        QEmbedMetaObjectPy::invoke_from_args(obj_ptr, method, retValue, args);
    }

    bool QEmbedMetaObjectPy::invoke_from_variant_list(uintptr_t obj_ptr, const std::string& method,
                                                      py::object& return_value, const py::iterable& args) {
        const auto obj = reinterpret_cast<QObject*>(static_cast<uintptr_t>(obj_ptr));
        if (!obj) {
            py::print("QEmbedMetaObjectPy.invoke_from_variant_list: null object pointer");
            return false;
        }

        std::vector<py::handle> pargs;
        for (auto a : args)
            pargs.push_back(a);

        const int argc = static_cast<int>(pargs.size());

        const QMetaObject* mo = obj->metaObject();
        QList<QMetaMethod> candidates;
        int returnTypeId = 0; // QMetaType id (int)
        for (int i = 0; i < mo->methodCount(); ++i) {
            QMetaMethod m = mo->method(i);
            if (m.methodType() != QMetaMethod::Method)
                continue;
            if (m.name() != QByteArray(method.c_str()))
                continue;
            if (m.parameterCount() != argc)
                continue;
            candidates.append(m);
        }

        QVariantList qargs;
        if (!candidates.isEmpty()) {
            const QMetaMethod& m = candidates.first();
            QList<QByteArray> paramTypes = m.parameterTypes();
            for (int i = 0; i < argc; ++i) {
                QByteArray expected = (i < paramTypes.size()) ? paramTypes[i] : QByteArray();
                if (expected.isEmpty()) {
                    qWarning() << "QEmbedMetaObjectPy.invoke_from_variant_list: empty expected type for argument"
                               << i << "of method" << QString::fromStdString(method);
                }
                auto converted = pyObjectToQVariant(pargs[i], expected);
                if (!converted.has_value()) {
                    qWarning()
                        << "QEmbedMetaObjectPy.invoke_from_variant_list: conversion to QVariant failed for argument"
                        << i << "of method" << QString::fromStdString(method)
                        << "expected type:" << QString::fromUtf8(expected);
                    return false;
                }
                qargs.push_back(converted.value());
            }
            returnTypeId = m.returnType();
        } else {
            for (auto& h : pargs) {
                auto converted = pyObjectToQVariant(h);
                if (!converted.has_value()) {
                    qWarning()
                        << "QEmbedMetaObjectPy.invoke_from_variant_list: conversion to QVariant failed for unknown argument";
                    return false;
                }
                qargs.push_back(converted.value());
            }
        }
        QVariant ret;
        bool ok = QEmbedMetaObject::invokeFromVariantListDynamic(obj, method.c_str(), qargs, &ret, Qt::AutoConnection);
        if (ok && !ret.isNull() && returnTypeId != QMetaType::Void) {
            return_value = qvariantToPyObject(ret);
        }
        return ok;
    }

    bool QEmbedMetaObjectPy::set_property(uintptr_t obj_ptr, const std::string& property_name, const py::object& value) {

        const auto obj = reinterpret_cast<QObject*>(static_cast<uintptr_t>(obj_ptr));
        if (!obj) {
            py::print("QEmbedMetaObjectPy.set_property: null object pointer");
            return false;
        }

        const QByteArray propName = QByteArray::fromStdString(property_name);
        const std::string propNameStr = propName.toStdString(); // для py::print
        const QMetaObject* mo = obj->metaObject();
        int idx = mo->indexOfProperty(propName.constData());

        if (idx >= 0) {
            QMetaProperty prop = mo->property(idx);
            QByteArray expectedType = prop.typeName() ? QByteArray(prop.typeName()) : QByteArray();
            auto v = pyObjectToQVariant(value, expectedType);
            if (!v.has_value() || !v.value().isValid()) {
                py::print("QEmbedMetaObjectPy.set_property: conversion to QVariant failed for property", propNameStr);
                return false;
            }
            if (!prop.isWritable()) {
                py::print("QEmbedMetaObjectPy.set_property: property not writable", propNameStr);
                return false;
            }
            if (!prop.write(obj, v.value())) {
                const std::string expectedTypeStr =
                    expectedType.isEmpty() ? std::string("<unknown>") : expectedType.toStdString();
                const char* vTypeC = v.value().typeName() ? v.value().typeName() : "<unknown>";
                std::string vTypeStr(vTypeC);
                std::string vStr;
                if (v.value().canConvert<QString>()) {
                    vStr = v.value().toString().toStdString();
                } else {
                    vStr = "<non-string>";
                }

                py::print("QEmbedMetaObjectPy.set_property: QMetaProperty::write failed for", propNameStr,
                          "expectedType=", expectedTypeStr, "qvariantType=", vTypeStr, "value=", vStr);
                return false;
            }
            return true;
        } else {
            // Свойство не найдено в метаобъекте — пробуем динамическое свойство через QObject::setProperty
            auto v = pyObjectToQVariant(value, QByteArray());
            if (!v.has_value() || !v.value().isValid()) {
                py::print("QEmbedMetaObjectPy.set_property: conversion to QVariant failed for dynamic property",
                          propNameStr);
                return false;
            }
            bool ok = obj->setProperty(propName.constData(), v.value());
            if (!ok) {
                py::print("QEmbedMetaObjectPy.set_property: QObject::setProperty failed for", propNameStr);
            }
            return ok;
        }
    }
    py::object QEmbedMetaObjectPy::get_property(uintptr_t obj_ptr, const std::string& property_name) {
        const auto obj = reinterpret_cast<QObject*>(static_cast<uintptr_t>(obj_ptr));
        if (!obj) {
            py::print("QEmbedMetaObjectPy.set_property: null object pointer");
            return py::none();
        }

        const QByteArray propName = QByteArray::fromStdString(property_name);
        const std::string propNameStr = propName.toStdString(); // для py::print
        const QMetaObject* mo = obj->metaObject();
        const int idx = mo->indexOfProperty(propName.constData());
        if (idx >= 0) {
            if (const QMetaProperty prop = mo->property(idx); prop.isReadable()) {
                const auto v = prop.read(obj);
                if (!v.isValid()) {
                    py::print("QEmbedMetaObjectPy.get_property: reading property returned invalid QVariant for",
                              propNameStr);
                    return py::none();
                }
                return qvariantToPyObject(v);
            }
            py::print("QEmbedMetaObjectPy.get_property: property not readable", propNameStr);
            return py::none();
        }
        const auto v = obj->property(propName.constData());
        if (!v.isValid()) {
            py::print("QEmbedMetaObjectPy.get_property: reading dynamic property returned invalid QVariant for",
                      propNameStr);
            return py::none();
        }
        return qvariantToPyObject(v);
    }

    static py::object invoke_returning(uintptr_t obj_ptr, const std::string& method, py::args args) {
        py::object return_value = py::none();
        if (const auto ok = QEmbedMetaObjectPy::invoke_from_args(obj_ptr, method, return_value, args);
            ok && return_value && !return_value.is_none()) {
            return return_value;
            }
        return py::none();
    }
} // namespace qtpyt