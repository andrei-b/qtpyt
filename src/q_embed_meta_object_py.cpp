#define PYBIND11_DETAILED_ERROR_MESSAGES
#include <pybind11/pybind11.h>
#include "q_embed_meta_object_py.h"
#include "conversions.h"
#include "q_embed_meta_object.h"
#include <QColor>
#include <QJsonArray>
#include <QJsonObject>
#include <QMetaMethod>
#include <QVariantList>
#include <qpoint.h>
#include <QString>

class QObject;
namespace qtpyt {
    uintptr_t find_object_by_name(const uintptr_t root_ptr, const std::string& name,
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
    bool invoke_from_args(const uintptr_t obj_ptr, const std::string& method, py::object& retValue,
                                              const py::args& args) {
        // forward to existing implementation which accepts a py::iterable
        return invoke_from_variant_list(obj_ptr, method, retValue,
                                                            py::reinterpret_borrow<py::iterable>(args));
    }


    void invoke_void_from_args_2(const uintptr_t obj_ptr, const std::string& method, const py::args& args) {
        py::object retValue; // unused
        invoke_from_args(obj_ptr, method, retValue, args);
    }

    py::object invoke_returning_from_args(const uintptr_t obj_ptr, const std::string &method, const py::args &args) {
        py::object retValue;
        invoke_from_args(obj_ptr, method, retValue, args);
        return retValue;
    }

    bool invoke_from_variant_list(uintptr_t obj_ptr, const std::string& method,
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

    bool set_property(uintptr_t obj_ptr, const std::string& property_name, const py::object& value) {
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
            obj->setProperty(propNameStr.c_str(), v.value());
            return true;
        }
         auto v = pyObjectToQVariant(value, QByteArray());
         if (!v.has_value() || !v.value().isValid()) {
                py::print("QEmbedMetaObjectPy.set_property: conversion to QVariant failed for dynamic property",
                          propNameStr);
                return false;
            }
            obj->setProperty(propNameStr.c_str(), v.value());
            return true;

    }

    bool set_property_async(uintptr_t obj_ptr, const std::string& property_name, const py::object& value) {
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
            QMetaObject::invokeMethod(obj, [obj, v, propNameStr]() {
                    obj->setProperty(propNameStr.c_str(), v.value());
                }, Qt::QueuedConnection);
            return true;
        }
        auto v = pyObjectToQVariant(value, QByteArray());
        if (!v.has_value() || !v.value().isValid()) {
             py::print("QEmbedMetaObjectPy.set_property: conversion to QVariant failed for dynamic property",
                          propNameStr);
           return false;
        }
        QMetaObject::invokeMethod(obj, [obj, v, propNameStr]() {
             obj->setProperty(propNameStr.c_str(), v.value());
        }, Qt::QueuedConnection);
        return true;
    }
    py::object get_property(uintptr_t obj_ptr, const std::string& property_name) {
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
        if (const auto ok = invoke_from_args(obj_ptr, method, return_value, args);
            ok && return_value && !return_value.is_none()) {
            return return_value;
            }
        return py::none();
    }
} // namespace qtpyt