#pragma once

#include  <qtpyt/pep3118format.h>
#include <qtpyt/conversions.h>
#include <QtCore/QSharedPointer>
#include <QtCore/QByteArray>
#include <QtCore/QtGlobal>

#include <cstring>
#include <type_traits>
#include <utility>
#include <algorithm>
#include <pybind11/pybind11.h>
namespace py = pybind11;

namespace qtpyt {
    template<typename T>
    class QPySharedArray {
    public:
        using value_type = T;
        using size_type = qsizetype;


        // ---- ctors ----
        QPySharedArray()
            : d_(QSharedPointer<Buffer>::create()) {
        }

        explicit QPySharedArray(size_type n);

        QPySharedArray(size_type n, const T &value)
            : QPySharedArray(n) {
            fill(value);
        }

        QPySharedArray(const QPySharedArray &other) {
            d_ = other.d_;
            m_readonly = other.m_readonly;
        }

        QPySharedArray(QPySharedArray &&other) noexcept {
            d_ = std::move(other.d_);

            m_readonly = other.m_readonly;
        }

        QPySharedArray &operator=(const QPySharedArray &other) {
            if (this != &other) {
                d_ = other.d_;
                m_readonly = other.m_readonly;
            }
            return *this;
        }

        QPySharedArray &operator=(QPySharedArray &&other) noexcept {
            if (this != &other) {
                d_ = std::move(other.d_);
                m_readonly = other.m_readonly;
            }
            return *this;
        }


        static QPySharedArray wrap(T *externalPtr, size_type n, bool takeOwnership = false) {
            QPySharedArray a;
            a.d_->resetToExternal(externalPtr, n, takeOwnership);
            return a;
        }

        // Copy: shares the same buffer header (same memory object)
        //QPySharedArray(const QPySharedArray&)            = default;
        //QPySharedArray& operator=(const QPySharedArray&) = default;

        // Move
        //QPySharedArray(QPySharedArray&&) noexcept            = default;
        //QPySharedArray& operator=(QPySharedArray&&) noexcept = default;

        ~QPySharedArray() = default;

        // ---- basic QByteArray-like API ----
        bool isEmpty() const noexcept { return size() == 0; }
        size_type size() const noexcept { return d_->size; }
        size_type capacity() const noexcept { return d_->capacity; }

        T *data() noexcept { return d_->ptr; }
        const T *constData() const noexcept { return d_->ptr; }
        const T *data() const noexcept { return d_->ptr; }

        void clear() { resize(0); }

        void resize(size_type n) {
            if (n < 0) n = 0;
            d_->ensureCapacity(n);

            if (n > d_->size) {
                defaultConstructRange(d_->ptr + d_->size, n - d_->size);
            } else if (n < d_->size) {
                destroyRange(d_->ptr + n, d_->size - n);
            }

            d_->size = n;
        }

        void reserve(size_type n) {
            if (n < 0) n = 0;
            d_->ensureCapacity(n);
        }

        void squeeze() {
            d_->shrinkToFit();
        }

        void fill(const T &v) {
            for (size_type i = 0; i < d_->size; ++i)
                d_->ptr[i] = v;
        }

        void append(const T &v) {
            const auto old = d_->size;
            resize(old + 1);
            d_->ptr[old] = v;
        }

        void append(const T *src, size_type n) {
            if (!src || n <= 0) return;
            const auto old = d_->size;
            resize(old + n);
            copyOrMemcpy(d_->ptr + old, src, n);
        }

        void append(const QPySharedArray &other) {
            append(other.constData(), other.size());
        }

        // QByteArray-style access (no bounds check)
        T &operator[](size_type i) { return d_->ptr[i]; }
        const T &operator[](size_type i) const { return d_->ptr[i]; }

        // Iteration
        T *begin() noexcept { return data(); }
        T *end() noexcept { return data() + size(); }
        const T *begin() const noexcept { return constData(); }
        const T *end() const noexcept { return constData() + size(); }
        const T *cbegin() const noexcept { return constData(); }
        const T *cend() const noexcept { return constData() + size(); }

        operator QVariant() const {
            return QVariant::fromValue(*this);
        }

        // Expose whether we are wrapping external memory
        bool isExternal() const noexcept { return d_->external; }

        // Convert to QByteArray for byte-like types (optional helper)
        template<class U = T>
        std::enable_if_t<std::is_same_v<U, char> || std::is_same_v<U, unsigned char> || std::is_same_v<U, std::byte>,
            QByteArray>
        toByteArrayView() const {
            return QByteArray::fromRawData(reinterpret_cast<const char *>(constData()),
                                           static_cast<int>(size() * sizeof(T)));
        }

        void setMemoryViewReadOnly(bool ro) noexcept {
            m_readonly = ro;
        }

        auto isMemoryViewReadOnly() const noexcept {
            return m_readonly;
        }

        [[nodiscard]] auto toMemoryView() const {
            static_assert(std::is_trivially_copyable_v<T>,
                          "Typed memoryview requires trivially copyable T");

            const py::ssize_t n = static_cast<py::ssize_t>(size());
            d_->format = pep3118::full_format_string<T>(false);

            py::gil_scoped_acquire gil;
            return py::memoryview::from_buffer(const_cast<void *>(static_cast<const void *>(constData())),
                                               static_cast<py::ssize_t>(sizeof(T)), // itemsize
                                               d_->format.c_str(),
                                               std::vector<py::ssize_t>{n}, // shape
                                               std::vector<py::ssize_t>{static_cast<py::ssize_t>(sizeof(T))},
                                               m_readonly);
        }


        auto toMemoryViewBytes(bool readonly = false) const {
            const py::ssize_t nbytes = static_cast<py::ssize_t>(size() * sizeof(T));
            auto *ptr = const_cast<unsigned char *>(
                reinterpret_cast<const unsigned char *>(constData())
            );

            return py::memoryview::from_memory(ptr, nbytes, readonly);
        }

    private:
        struct Buffer {
            T *ptr = nullptr;
            size_type size = 0;
            size_type capacity = 0;

            bool external = false;
            bool takeOwnership = false;
            py::object owner;
            std::string format;

            Buffer() = default;

            Buffer(Buffer &&) = delete;

            Buffer &operator=(Buffer &&) = delete;

            Buffer(const Buffer &) = delete;

            Buffer &operator=(const Buffer &) = delete;

            ~Buffer() {
                release();
            }

            void resetToExternal(T *p, size_type n, bool takeOwn) {
                release();
                ptr = p;
                size = (n < 0 ? 0 : n);
                capacity = size;
                external = true;
                takeOwnership = takeOwn;
            }

            void resetToExternalWithOwner(T *p, size_type n, bool takeOwn, py::object &&ownerIn) {
                release();
                ptr = p;
                size = (n < 0 ? 0 : n);
                capacity = size;
                external = true;
                takeOwnership = takeOwn;
                // store owner to keep Python object alive (may be a memoryview/buffer)
                owner = std::move(ownerIn);
                (void) owner.inc_ref();
            }

            void ensureCapacity(size_type required) {
                if (required <= capacity) return;

                // External buffers cannot be grown in-place unless we "adopt" (copy) into owned memory.
                if (external) {
                    // Adopt: allocate owned memory and copy over. Still shared across all instances (same header).
                    allocateOwned(required);
                    external = false;
                    takeOwnership = true; // now we own
                    return;
                }

                growOwned(required);
            }

            void shrinkToFit() {
                if (external) {
                    // Can't shrink external pointer without adopting.
                    return;
                }

                if (size == capacity) return;
                if (size == 0) {
                    release();
                    return;
                }

                T *newPtr = allocateRaw(size);
                moveOrMemcpy(newPtr, ptr, size);
                destroyRange(ptr, size);
                deallocateRaw(ptr);

                ptr = newPtr;
                capacity = size;
            }

            void release() {
                if (!ptr) {
                    size = 0;
                    capacity = 0;
                    external = false;
                    takeOwnership = false;
                    // clear owner if present
                    if (!owner.is_none()) {
                        py::gil_scoped_acquire gil;
                        owner = py::object();
                    }
                    return;
                }

                if (external) {
                    if (takeOwnership) {
                        // destroy and delete[] as before
                        destroyRange(ptr, size);
                        delete[] ptr;
                    }
                    ptr = nullptr;
                    size = 0;
                    capacity = 0;
                    external = false;
                    takeOwnership = false;
                    if (!owner.is_none()) {
                        py::gil_scoped_acquire gil;
                        owner = py::object();
                    }
                    return;
                }

                // Owned
                destroyRange(ptr, size);
                delete[] ptr;

                ptr = nullptr;
                size = 0;
                capacity = 0;
                external = false;
                takeOwnership = false;
                if (!owner.is_none()) {
                    py::gil_scoped_acquire gil;
                    owner = py::object();
                }
            }

            void allocateOwned(size_type required) {
                // Allocate new owned storage and copy/move current content
                const size_type newCap = std::max<size_type>(required, nextCapacity(capacity, required));

                T *newPtr = allocateRaw(newCap);
                if (ptr && size > 0) {
                    moveOrMemcpy(newPtr, ptr, size);
                    // For external-but-not-owned, we must NOT destroy old content.
                    // For external-and-owned, we'd have released earlier anyway.
                }

                ptr = newPtr;
                capacity = newCap;
            }

            void growOwned(size_type required) {
                const size_type newCap = nextCapacity(capacity, required);
                T *newPtr = allocateRaw(newCap);

                if (ptr && size > 0) {
                    moveOrMemcpy(newPtr, ptr, size);
                    destroyRange(ptr, size);
                    deallocateRaw(ptr);
                }

                ptr = newPtr;
                capacity = newCap;
            }

            static size_type nextCapacity(size_type current, size_type required) {
                // QByteArray-ish growth (roughly 1.5x / 2x), keep it simple
                size_type cap = (current > 0 ? current : 1);
                while (cap < required) {
                    cap = cap + cap / 2 + 8; // grow ~1.5x + small constant
                    if (cap < 0) {
                        // overflow guard
                        cap = required;
                        break;
                    }
                }
                return cap;
            }

            static T *allocateRaw(size_type n) {
                // new[] value-initializes; we want uninitialized storage sometimes.
                // But for simplicity/safety, allocate as new T[n] and treat constructedness in helpers.
                // We'll still explicitly destroy for non-trivial types when shrinking/resizing down.
                return (n > 0) ? new T[static_cast<size_t>(n)] : nullptr;
            }

            static void deallocateRaw(T *p) { delete[] p; }
        };

        static void defaultConstructRange(T *p, size_type n) {
            if (!p || n <= 0) return;

            if constexpr (std::is_trivially_default_constructible_v<T>) {
                // nothing required (new[] already default-initialized storage in this implementation)
                // We leave as-is.
            } else {
                for (size_type i = 0; i < n; ++i) {
                    (void) p[i];
                }
            }
        }

        static void destroyRange(T *p, size_type n) {
            if (!p || n <= 0) return;

            if constexpr (!std::is_trivially_destructible_v<T>) {
                for (size_type i = 0; i < n; ++i)
                    p[i].~T();
            }
            // If trivially destructible, nothing.
        }

        static void copyOrMemcpy(T *dst, const T *src, size_type n) {
            if (!dst || !src || n <= 0) return;
            if constexpr (std::is_trivially_copyable_v<T>) {
                std::memcpy(dst, src, static_cast<size_t>(n) * sizeof(T));
            } else {
                std::copy(src, src + n, dst);
            }
        }

        static void moveOrMemcpy(T *dst, T *src, size_type n) {
            if (!dst || !src || n <= 0) return;

            if constexpr (std::is_trivially_copyable_v<T>) {
                std::memcpy(dst, src, static_cast<size_t>(n) * sizeof(T));
            } else {
                std::move(src, src + n, dst);
            }
        }

    public:
        static QPySharedArray wrapFromPyBuffer(const py::buffer &buf, bool takeOwnership = false) {
            if (!buf || buf.is_none())
                return QPySharedArray();

            py::buffer_info info = buf.request();
            // require contiguous 1-D buffer with matching itemsize
            if (info.ndim != 1 || static_cast<size_t>(info.itemsize) != sizeof(T))
                return QPySharedArray();

            T *ptr = static_cast<T *>(info.ptr);
            const size_type n = static_cast<size_type>(info.size);

            QPySharedArray a;
            // Keep the python buffer/memoryview alive by passing buf as owner
            a.d_->resetToExternalWithOwner(ptr, n, takeOwnership, std::move(py::reinterpret_borrow<py::object>(buf)));
            return a;
        }

    private:
        QSharedPointer<Buffer> d_;
        bool m_readonly{false};
        //mutable std::string m_formatString;
    };

    template<class T>
    QPySharedArray<T>::QPySharedArray(size_type n) : QPySharedArray() {
        resize(n);
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
                QPySharedArray<T> a = QPySharedArray<T>::wrapFromPyBuffer(buf, /*takeOwnership=*/false);
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
                QPySharedArray<T> a = QPySharedArray<T>::wrapFromPyBuffer(buf, /*takeOwnership=*/false);
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
            const auto *ptr = static_cast<const QPySharedArray<T> *>(v);
            return ptr->toMemoryView();
        });
        addFromQVariantFunc(id, [](const QVariant &v) {
            QPySharedArray<T> arr = v.template value<QPySharedArray<T> >();
            return arr.toMemoryView();
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

