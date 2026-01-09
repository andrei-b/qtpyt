#include "conversions.h"
#include "../include/qtpyt/qpyconvertiblepointer.h"
#include "internal/stringpool.h"

namespace qtpyt {
    void registerConvertiblePointerType() {
        int id = qRegisterMetaType<QPyConvertiblePointer>("QPyConvertiblePointer");
        addMetatypeVoidPtrToPyObjectConverterFunc(static_cast<QMetaType::Type>(id), [](const void* v) {
            auto* ptr = static_cast<QPyConvertiblePointer*>(const_cast<void*>(v));
            py::gil_scoped_acquire gil;
            py::memoryview mv = py::memoryview::from_buffer(
                ptr->getPointer(),
                static_cast<py::ssize_t>(ptr->itemSize()),
                std::string(1, ptr->format()).c_str(),
                { static_cast<py::ssize_t>(ptr->itemCount()) },         // shape
                { static_cast<py::ssize_t>(ptr->itemSize()) },
                ptr->isReadOnly()
            );
            return mv;
        });

        addFromQVariantFunc(id, [](const QVariant& v) {
            auto cp = v.value<QPyConvertiblePointer>();
            py::gil_scoped_acquire gil;
            py::memoryview mv = py::memoryview::from_buffer(
                cp.getPointer(),
                static_cast<py::ssize_t>(cp.itemSize()),
                StringPool::instance().intern(std::string(1, cp.format()))->c_str(),
                { static_cast<py::ssize_t>(cp.itemCount()) },         // shape
                { static_cast<py::ssize_t>(cp.itemSize()) },
                cp.isReadOnly()
            );
            return mv;
        });

        QString typeName = QMetaType::typeName(id);
        addFromPyObjectToQVariantFunc(typeName, [](const py::object& obj) -> QVariant {
            py::gil_scoped_acquire gil;
            auto info = py::buffer(py::reinterpret_borrow<py::buffer>(obj)).request();
            const QPyConvertiblePointer cp(
                [obj, pointer = info.ptr]() {
                    auto t_obj = py::reinterpret_borrow<py::object>(obj);
                    return pointer;
                },
                StringPool::instance().intern(std::string(1, info.format[0]))->c_str()[0],
                static_cast<std::size_t>(info.itemsize),
                info.size,
                info.readonly);
            return QVariant::fromValue(cp);
        });
    }
} // qtpyt

Q_DECLARE_METATYPE(qtpyt::QPyConvertiblePointer)