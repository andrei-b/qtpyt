#pragma once
#ifdef slots
  #pragma push_macro("slots")
  #undef slots
  #define _RESTORE_SLOTS 1
#endif
#include <pybind11/pybind11.h>
#include <pybind11/buffer_info.h>
#include <pybind11/numpy.h>
#ifdef _RESTORE_SLOTS
  #pragma pop_macro("slots")
  #undef _RESTORE_SLOTS
#endif
#include <qtpyt/qpysharedarray.h>
#include "../conversions.h"
#include <QVariant>
#include <vector>

namespace qtpyt {
namespace py = pybind11;

inline std::shared_ptr<detail::OwnerState> keep_alive(py::object obj) {
    auto* heapObj = new py::object(std::move(obj));
    return detail::make_owner(
        heapObj,
        [](void* p) {
            // Deleting py::object will DECREF; ensure GIL is held.
            py::gil_scoped_acquire gil;
            delete static_cast<py::object*>(p);
        }
    );
}

// Convert QPySharedArray<T> -> memoryview (zero-copy if external/owned contiguous)
template <typename T>
py::memoryview to_memoryview( qtpyt::QPySharedArray<T>* a) {
    py::gil_scoped_acquire gil;
    const py::ssize_t n = static_cast<py::ssize_t>(a->size());

    // 1D buffer
    return py::memoryview::from_buffer(a->data(), // ptr
                                       static_cast<py::ssize_t>(sizeof(T)), // itemsize
                                       py::format_descriptor<T>::format().c_str(),
                                       std::vector<py::ssize_t>{n}, // shape
                                       std::vector<py::ssize_t>{static_cast<py::ssize_t>(sizeof(T))},
                                       a->isReadOnly());
}

// Convert Python buffer -> QPySharedArray<T>, optionally zero-copy by viewing exporter memory
template <typename T>
qtpyt::QPySharedArray<T> from_buffer(py::buffer b, bool allowZeroCopy = true, bool takeOwnership = false) {
    py::gil_scoped_acquire gil;
    py::buffer_info info = b.request();

    if (info.ndim != 1)
        throw std::runtime_error("Expected a 1D buffer");

    if (info.itemsize != (py::ssize_t)sizeof(T))
        throw std::runtime_error("Itemsize mismatch for requested T");

    // If you want strict format checking, compare info.format vs format_descriptor<T>::format()
    // (Some exporters leave format empty or use compatible aliases.)
    const auto n = static_cast<qsizetype>(info.shape[0]);
    auto* ptr = static_cast<T*>(info.ptr);

    if (allowZeroCopy) {
        // attach owner: keep the original buffer object alive
        auto owner = keep_alive(py::reinterpret_borrow<py::object>(b));
        return qtpyt::QPySharedArray<T>::wrapWithOwner(ptr, n, takeOwnership, std::move(owner));
    }

    qtpyt::QPySharedArray<T> out(n);
    std::memcpy(out.data(), ptr, size_t(n) * sizeof(T));
    return out;
}

 template<typename T>
    QVariant pyObjectToQVariantSharedArray(const py::object &obj, bool allowZeroCopy = true) {
        if (!obj || obj.is_none()) {
            return QVariant();
        }

        auto copy_bytes_to_array = [](const void *src, size_t nbytes) -> QVariant {
            if (nbytes == 0) {
                QPySharedArray<T> empty;
                return QVariant::fromValue(empty);
            }
            if (nbytes % sizeof(T) != 0)
                return QVariant(); // size mismatch

            const qsizetype nelems = static_cast<qsizetype>(nbytes / sizeof(T));
            QPySharedArray<T> arr(nelems);
            std::memcpy(arr.data(), src, nbytes);
            return QVariant::fromValue(arr);
        };

        if (py::isinstance<py::buffer>(obj)) {
            py::buffer buf = py::buffer(obj);
            py::buffer_info info = buf.request();

            if (allowZeroCopy && info.ndim == 1 && static_cast<size_t>(info.itemsize) == sizeof(T)) {
                QPySharedArray<T> a = from_buffer<T>(buf, allowZeroCopy, /*takeOwnership=*/false);
                return QVariant::fromValue(a);
            }

            const size_t total_bytes = static_cast<size_t>(info.size) * static_cast<size_t>(info.itemsize);
            if (static_cast<size_t>(info.itemsize) == sizeof(T)) {
                return copy_bytes_to_array(info.ptr, total_bytes);
            }
        }

        if (py::isinstance<py::bytes>(obj) || py::isinstance<py::bytearray>(obj)) {
            py::buffer buf = py::buffer(obj);
            py::buffer_info info = buf.request();
            const size_t total_bytes = static_cast<size_t>(info.size) * static_cast<size_t>(info.itemsize);

            if (allowZeroCopy && info.ndim == 1 && static_cast<size_t>(info.itemsize) == sizeof(T)) {
                QPySharedArray<T> a = from_buffer<T>(buf, allowZeroCopy, /*takeOwnership=*/false);
                return QVariant::fromValue(a);
            }

            if (static_cast<size_t>(info.itemsize) == sizeof(T)) {
                return copy_bytes_to_array(info.ptr, total_bytes);
            }
            return QVariant();
        }

        if (py::isinstance<py::sequence>(obj)) {
            py::sequence seq = py::reinterpret_borrow<py::sequence>(obj);
            const ssize_t n = seq.size();
            if (n < 0) return QVariant();

            QPySharedArray<T> arr(static_cast<qsizetype>(n));
            for (ssize_t i = 0; i < n; ++i) {
                arr[static_cast<qsizetype>(i)] = seq[i].cast<T>();
            }
            return QVariant::fromValue(arr);
        }

        // Unknown type -> fail
        return QVariant();
    }


    template<typename T>
    static int registerSharedArray(const QString &name, bool allowZeroCopy = true) {
        auto id = qRegisterMetaType<QPySharedArray<T> >(name.toStdString().c_str());
        addMetatypeVoidPtrToPyObjectConverterFunc(static_cast<QMetaType::Type>(id), [](const void *v) {
            auto *ptr = static_cast<QPySharedArray<T> *>(const_cast<void*>(v));
            return to_memoryview<T>(ptr);
        });

        addFromQVariantFunc(id, [](const QVariant &v) {
            QPySharedArray<T> arr = v.template value<QPySharedArray<T> >();
            auto obj = to_memoryview<T>(&arr);
            return obj;
        });

    QString typeName = QMetaType::typeName(id);
        addFromPyObjectToQVariantFunc(typeName, [allowZeroCopy](const py::object &obj) -> QVariant {
            return pyObjectToQVariantSharedArray<T>(obj, allowZeroCopy);
        });
        return id;
    }
} // namespace qtpyt

Q_DECLARE_METATYPE(qtpyt::QPySharedArray<char>)

Q_DECLARE_METATYPE(qtpyt::QPySharedArray<unsigned char>)

Q_DECLARE_METATYPE(qtpyt::QPySharedArray<short>)

Q_DECLARE_METATYPE(qtpyt::QPySharedArray<unsigned short>)

Q_DECLARE_METATYPE(qtpyt::QPySharedArray<int>)

Q_DECLARE_METATYPE(qtpyt::QPySharedArray<unsigned int>)

Q_DECLARE_METATYPE(qtpyt::QPySharedArray<long>)

Q_DECLARE_METATYPE(qtpyt::QPySharedArray<unsigned long>)

Q_DECLARE_METATYPE(qtpyt::QPySharedArray<long long>)

Q_DECLARE_METATYPE(qtpyt::QPySharedArray<unsigned long long>)

Q_DECLARE_METATYPE(qtpyt::QPySharedArray<float>)

Q_DECLARE_METATYPE(qtpyt::QPySharedArray<double>)

Q_DECLARE_METATYPE(qtpyt::QPySharedArray<std::byte>)

Q_DECLARE_METATYPE(qtpyt::QPySharedArray<std::int8_t>)

