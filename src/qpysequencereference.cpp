#include "conversions.h"
#include "../include/qtpyt/qpysequencereference.h"
#include "internal/stringpool.h"

namespace qtpyt {
    void registerSequenceReferenceType() {
        int id = qRegisterMetaType<QPySequenceReference>("QPySequenceReference");
        addMetatypeVoidPtrToPyObjectConverterFunc(static_cast<QMetaType::Type>(id), [](const void* v) {
            auto* ptr = static_cast<QPySequenceReference*>(const_cast<void*>(v));
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
    auto cp = v.value<QPySequenceReference>();
    py::gil_scoped_acquire gil;

    // Create a capsule that keeps cp alive
    auto* cp_ptr = new QPySequenceReference(cp);
    auto capsule = py::capsule(cp_ptr, [](void* ptr) {
        delete static_cast<QPySequenceReference*>(ptr);
    });

    // Create buffer info
    Py_buffer view;
    view.buf = cp.getPointer();
    view.obj = capsule.ptr();
    view.len = static_cast<Py_ssize_t>(cp.itemCount() * cp.itemSize());
    view.itemsize = static_cast<Py_ssize_t>(cp.itemSize());
    view.readonly = cp.isReadOnly() ? 1 : 0;
    view.ndim = 1;
    view.format = const_cast<char*>(StringPool::instance().intern(std::string(1, cp.format()))->c_str());
    view.shape = new Py_ssize_t[1]{static_cast<Py_ssize_t>(cp.itemCount())};
    view.strides = new Py_ssize_t[1]{static_cast<Py_ssize_t>(cp.itemSize())};
    view.suboffsets = nullptr;
    view.internal = nullptr;

    PyObject* mv_obj = PyMemoryView_FromBuffer(&view);

    delete[] view.shape;
    delete[] view.strides;

    return py::reinterpret_steal<py::memoryview>(mv_obj);
});
                
        QString typeName = QMetaType::typeName(id);
        addFromPyObjectToQVariantFunc(typeName, [](const py::object& obj) -> QVariant {
            py::gil_scoped_acquire gil;
            auto info = py::buffer(py::reinterpret_borrow<py::buffer>(obj)).request();
            auto shared_obj = std::shared_ptr<py::object>(
     new py::object(obj),
     [](py::object* ptr) {
         pybind11::gil_scoped_acquire gil;
         delete ptr;  // py::object destructor runs with GIL held
     }
 );
            const QPySequenceReference cp(
                [shared_obj, pointer = info.ptr]() {
                    pybind11::gil_scoped_acquire gil;
                    shared_obj.get()->inc_ref();
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

Q_DECLARE_METATYPE(qtpyt::QPySequenceReference)