// File: `include/qtpyt/qpysharedarray.h`
/**
 * @file qpysharedarray.h
 * @brief Lightweight vector-like container that allows to pass data between Qt C++ and Python without copying.
 * Python functions receive and return the values of this type as memoryview objects.
 */

#pragma once

#include <QtCore/QSharedPointer>
#include <QtCore/QByteArray>
#include <QtCore/QtGlobal>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <type_traits>
#include <utility>
#include <algorithm>
#include <QMap>
#include <QVariant>

namespace qtpyt {

namespace detail {

/**
 * @struct OwnerState
 * @brief Helper that holds a void pointer and a deleter to manage external lifetime.
 */
struct OwnerState {
    void* estate = nullptr;               ///< pointer to external state (opaque)
    void (*deleter)(void*) = nullptr;     ///< custom deleter for the external state

    OwnerState() = default;
    OwnerState(void* c, void (*d)(void*)) : estate(c), deleter(d) {}
    ~OwnerState() { if (deleter) deleter(estate); }

    OwnerState(const OwnerState&) = delete;
    OwnerState& operator=(const OwnerState&) = delete;
};


inline std::shared_ptr<OwnerState> make_owner(void* estate, void (*deleter)(void*)) {
    return std::make_shared<OwnerState>(estate, deleter);
}

} // namespace detail


class external; ///< forward declaration for external usage in other headers

/**
 * @class QPySharedArray
 * @brief A shared data container that can be treated as a vector of T on the C++ side
 * and as a memoryview the Python side.
 *
 * Template parameter \c T is the element type.
 *
 * Key features:
 * - Supports wrapping external buffers without copying.
 * - Optionally takes ownership of external buffers or keeps an external owner alive
 *   via a \c std::shared_ptr<detail::OwnerState>.
 *   Preserves buffer lifetime across C++/Python boundary.
 * - Provides detach() semantics: modifying operations create a private copy.
 *
 * @tparam T Element type.
 */
template <typename T>
class QPySharedArray {
public:
    using value_type = T;
    using size_type  = qsizetype;

    /**
     * @brief Construct an empty QPySharedArray.
     */
    QPySharedArray() : d_(QSharedPointer<Data>::create()) {}

    /**
     * @brief Construct with \p n default-initialized elements.
     * @param n Number of elements to reserve/resize to.
     */
    explicit QPySharedArray(size_type n) : d_(QSharedPointer<Data>::create()) { resize(n); }

    /**
     * @brief Wrap an external buffer as a QPySharedArray view.
     *
     * @param externalPtr Pointer to external element data.
     * @param n Number of elements pointed to by \p externalPtr.
     * @param takeOwnership If true, the array will delete[] \p externalPtr when owning Data is destroyed.
     * @return QPySharedArray<T> view over the external data.
     */
    static QPySharedArray wrap(T* externalPtr, size_type n, bool takeOwnership = false) {
        QPySharedArray a;
        a.d_->resetToExternal(externalPtr, n, takeOwnership, /*owner*/nullptr);
        return a;
    }

    /**
     * @brief Wrap an external buffer and keep an owner object alive alongside it.
     *
     * Use this when the external buffer lifetime is tied to some other object;
     * provide an OwnerState shared_ptr to ensure that object stays alive.
     *
     * @param externalPtr Pointer to external element data.
     * @param n Number of elements.
     * @param takeOwnership If true, Data will delete[] the buffer when appropriate.
     * @param owner Shared owner state that will be kept alive while the view exists.
     * @return QPySharedArray<T> view over the external data with shared owner.
     */
    static QPySharedArray wrapWithOwner(
        T* externalPtr,
        size_type n,
        bool takeOwnership,
        std::shared_ptr<detail::OwnerState> owner)
    {
        QPySharedArray a;
        a.d_->resetToExternal(externalPtr, n, takeOwnership, std::move(owner));
        return a;
    }

    /**
     * @brief Check whether the array is empty.
     * @return True if size() == 0.
     */
    bool isEmpty() const { return size() == 0; }

    /**
     * @brief Number of elements currently stored/referenced.
     * @return Element count.
     */
    size_type size() const { return d_->m_size; }

    /**
     * @brief Current capacity in elements.
     * @return Capacity.
     */
    size_type capacity() const { return d_->m_capacity; }

    /**
     * @brief Const pointer to the element data.
     * @return Pointer to first element (may point to external storage).
     */
    const T* constData() const { return d_->ptr(); }

    /**
     * @brief Mutable access to element data; detaches if necessary (copy-on-write).
     * @return Pointer to first element.
     */
    T* data() { detach(); return d_->ptr(); }

    /**
     * @brief Const element access with bounds assertion.
     * @param i Element index (0-based).
     * @return Const reference to element.
     */
    const T& operator[](size_type i) const { Q_ASSERT(i >= 0 && i < size()); return constData()[i]; }

    /**
     * @brief Mutable element access with detach semantics and bounds assertion.
     * @param i Element index (0-based).
     * @return Reference to element.
     */
    T& operator[](size_type i) { Q_ASSERT(i >= 0 && i < size()); return data()[i]; }

    /**
     * @brief Clear the array (resize to 0).
     */
    void clear() { resize(0); }

    /**
     * @brief Resize the array to \p n elements (may reallocate).
     * @param n New size in elements.
     */
    void resize(size_type n) {
        if (n == d_->m_size) return;
        detach();
        d_->resize(n);
    }

    /**
     * @brief Ensure capacity for at least \p n elements.
     * @param n Desired capacity in elements.
     */
    void reserve(size_type n) {
        if (n <= d_->m_capacity) return;
        detach();
        d_->reserve(n);
    }

    /**
     * @brief Detach from shared data (copy the Data object).
     *
     * This implements Qt-like detach semantics: subsequent modifications will
     * operate on a private copy of the data.
     */
    void detach() {
                d_ = QSharedPointer<Data>::create(*d_);
    }

    /**
     * @brief Query whether the underlying storage is read-only.
     * When set to true, the corresponding memoryview in Python will be read-only.
     * @return True if the array is read-only.
     */
    bool isReadOnly() const {
        return d_->m_readonly;
    }

    /**
     * @brief Mark the array read-only. Detaches first so this affects only this instance.
     * When set to true, the corresponding memoryview in Python will be read-only.
     * @param r True to mark read-only.
     */
    void setReadOnly(bool r) {
        detach();
        d_->m_readonly = r;
    }

    /**
     * @brief Convert to QVariant for easy use with Qt APIs.
     * @return QVariant holding a copy of this QPySharedArray.
     */
    operator QVariant() const {
        return QVariant::fromValue(*this);
    }




private:

    /**
     * @brief Internal storage object that holds either owned bytes or an external view.
     *
     */
    struct Data {

        QByteArray owned;                             ///< owned bytes (when not external)
        T* extPtr = nullptr;                          ///< external pointer when m_external is true

        size_type m_size = 0;                         ///< logical element count
        size_type m_capacity = 0;                     ///< capacity in elements

        bool m_external{false};                       ///< true when using extPtr
        bool m_takeOwnership{false};                  ///< if true, delete[] extPtr on destruction
        bool m_readonly{false};                       ///< read-only flag

        std::shared_ptr<detail::OwnerState> owner;    ///< optional owner that keeps external source alive

        Data() = default;

        /**
         * @brief Copy constructor: copies view/owner semantics for external buffers.
         */
        Data(const Data& o)
            : owned(o.owned),
              extPtr(o.extPtr),
              m_size(o.m_size),
              m_capacity(o.m_capacity),
              m_external(o.m_external),
              m_takeOwnership(o.m_takeOwnership),
              owner(o.owner)
        {}

        ~Data() {
            if (m_external && m_takeOwnership && extPtr) {
                delete[] extPtr;
            }
        }

        /**
         * @brief Pointer to element storage (owned or external).
         * @return Pointer to first element.
         */
        T* ptr() {
            return m_external ? extPtr : reinterpret_cast<T*>(owned.data());
        }
        const T* ptr() const {
            return m_external ? extPtr : reinterpret_cast<const T*>(owned.constData());
        }

        /**
         * @brief Reset this Data to reference an external buffer.
         * @param p External pointer.
         * @param n Number of elements.
         * @param takeOwn If true, Data will take ownership (delete[]) when appropriate.
         * @param keepAlive Optional owner shared_ptr to keep external owner alive.
         */
        void resetToExternal(T* p, size_type n, bool takeOwn, std::shared_ptr<detail::OwnerState> keepAlive) {
            owned.clear();
            owned.squeeze();
            m_external = true;
            extPtr = p;
            m_size = n;
            m_capacity = n;
            m_takeOwnership = takeOwn;
            owner = std::move(keepAlive);
        }

        /**
         * @brief Ensure owned storage for at least \p cap elements.
         *
         * If currently external, this will allocate a QByteArray, copy existing data
         * and drop the external view and its owner.
         *
         * @param cap Desired capacity in elements.
         */
        void ensureOwnedStorage(size_type cap) {
            if (!m_external) {
                if (owned.size() < cap * (int)sizeof(T))
                    owned.resize(int(cap * sizeof(T)));
                m_capacity = cap;
                return;
            }

            // external -> allocate owned and copy
            QByteArray b;
            b.resize(int(cap * sizeof(T)));
            if (extPtr && m_size > 0)
                std::memcpy(b.data(), extPtr, size_t(m_size) * sizeof(T));
            owned = std::move(b);

            // drop external view; owner may still exist but no longer needed
            m_external = false;
            extPtr = nullptr;
            m_takeOwnership = false;
            owner.reset();
            m_capacity = cap;
        }

        /**
         * @brief Reserve capacity for at least \p cap elements.
         * @param cap Desired capacity.
         */
        void reserve(size_type cap) {
            if (cap <= m_capacity) return;
            ensureOwnedStorage(cap);
        }

        /**
         * @brief Resize logical element count to \p n.
         *
         * When increasing beyond capacity, capacity grows (at least doubled).
         * If storage is owned, the underlying QByteArray is resized appropriately.
         *
         * @param n New size in elements.
         */
        void resize(size_type n) {
            if (n > m_capacity) reserve(std::max(n, m_capacity * 2));
            m_size = n;
            if (!m_external) {
                // keep QByteArray m_size consistent (so data() is valid)
                owned.resize(int(m_capacity * sizeof(T)));
            }
        }
    };

    QSharedPointer<Data> d_; ///< shared data pointer
};

} // namespace qtpyt
