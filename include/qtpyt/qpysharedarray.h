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


struct OwnerState {
    void* estate = nullptr;
    void (*deleter)(void*) = nullptr;

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


class external;
template <typename T>
class QPySharedArray {
public:
    using value_type = T;
    using size_type  = qsizetype;

    QPySharedArray() : d_(QSharedPointer<Data>::create()) {}
    explicit QPySharedArray(size_type n) : d_(QSharedPointer<Data>::create()) { resize(n); }

    static QPySharedArray wrap(T* externalPtr, size_type n, bool takeOwnership = false) {
        QPySharedArray a;
        a.d_->resetToExternal(externalPtr, n, takeOwnership, /*owner*/nullptr);
        return a;
    }

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

    bool isEmpty() const { return size() == 0; }
    size_type size() const { return d_->m_size; }
    size_type capacity() const { return d_->m_capacity; }

    const T* constData() const { return d_->ptr(); }
    T* data() { detach(); return d_->ptr(); }

    const T& operator[](size_type i) const { Q_ASSERT(i >= 0 && i < size()); return constData()[i]; }
    T& operator[](size_type i) { Q_ASSERT(i >= 0 && i < size()); return data()[i]; }

    void clear() { resize(0); }

    void resize(size_type n) {
        if (n == d_->m_size) return;
        detach();
        d_->resize(n);
    }

    void reserve(size_type n) {
        if (n <= d_->m_capacity) return;
        detach();
        d_->reserve(n);
    }

    // Detach semantics like QByteArray/QVector: if shared, copy buffer
    void detach() {
                d_ = QSharedPointer<Data>::create(*d_);
    }

    bool isReadOnly() const {
        return d_->m_readonly;
    }

    void setReadOnly(bool r) {
        detach();
        d_->m_readonly = r;
    }

    operator QVariant() const {
        return QVariant::fromValue(*this);
    }




private:

    struct Data {

        QByteArray owned;
        T* extPtr = nullptr;

        size_type m_size = 0;
        size_type m_capacity = 0;

        bool m_external{false};
        bool m_takeOwnership{false};
        bool m_readonly{false};

        std::shared_ptr<detail::OwnerState> owner; // keeps external source alive

        Data() = default;

        // Copy: if external, copy "view" + owner (shared lifetime)
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

        T* ptr() {
            return m_external ? extPtr : reinterpret_cast<T*>(owned.data());
        }
        const T* ptr() const {
            return m_external ? extPtr : reinterpret_cast<const T*>(owned.constData());
        }

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

        void reserve(size_type cap) {
            if (cap <= m_capacity) return;
            ensureOwnedStorage(cap);
        }

        void resize(size_type n) {
            if (n > m_capacity) reserve(std::max(n, m_capacity * 2));
            m_size = n;
            if (!m_external) {
                // keep QByteArray m_size consistent (so data() is valid)
                owned.resize(int(m_capacity * sizeof(T)));
            }
        }
    };

    QSharedPointer<Data> d_;
};

} // namespace qtpyt
