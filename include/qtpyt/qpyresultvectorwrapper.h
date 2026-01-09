#pragma once
#include "qpysequencereference.h"

namespace qtpyt {
    template<typename T>
    class QPyResultVectorWrapper {
    public:

        explicit QPyResultVectorWrapper(const QPySequenceReference& vec)
            : m_vector(vec) {
            if (sizeof(T) != m_vector.itemSize()) {
                throw std::invalid_argument("QPyResultVectorWrapper: Size of T does not match item size of QPyConvertiblePointer");
            }
        }

        T operator [](int idx) {
            T* dataPtr = static_cast<T*>(m_vector.getPointer());
            return dataPtr[idx];
        }

        std::size_t size() const noexcept {
            return m_vector.itemCount();
        }

        T at(std::size_t idx) {
            T* dataPtr = static_cast<T*>(m_vector.getPointer());
            if (idx >= m_vector.itemCount()) {
                throw std::out_of_range("Index out of range in QPyResultVectorWrapper::at");
            }
            return dataPtr[idx];
        }

        T *data() noexcept {
            return static_cast<T*>(m_vector.getPointer());
        }

        auto begin() noexcept {
            return data();
        }

        auto end() noexcept {
            return data() + size();
        }

    private:
        QPySequenceReference m_vector;
    };
} // namespace qtpyt
