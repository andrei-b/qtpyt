#pragma once
#include "qpymemoryview.h"

namespace qtpyt {

    template <typename T>
    constexpr char format_char() {
        if constexpr (std::is_same<T, std::int8_t>::value) {
            return 'b';
        } else if constexpr (std::is_same<T, std::uint8_t>::value) {
            return 'B';
        } else if constexpr (std::is_same<T, std::int16_t>::value) {
            return 'h';
        } else if constexpr (std::is_same<T, std::uint16_t>::value) {
            return 'H';
        } else if constexpr (std::is_same<T, std::int32_t>::value) {
            return 'i';
        } else if constexpr (std::is_same<T, std::uint32_t>::value) {
            return 'I';
        } else if constexpr (std::is_same<T, std::int64_t>::value) {
            return 'q';
        } else if constexpr (std::is_same<T, std::uint64_t>::value) {
            return 'Q';
        } else if constexpr (std::is_same<T, float>::value) {
            return 'f';
        } else if constexpr (std::is_same<T, double>::value) {
            return 'd';
        } else if constexpr (std::is_same<T, bool>::value) {
            return '?';
        } else {
            static_assert(sizeof(T) == 0, "Unsupported type for QPyArray");
        }
    }

    template <typename T>
    class QPyArray : public QPyMemoryView {
    public:
        QPyArray(T* ptr, std::size_t itemCount, bool readOnly = false)
            : QPyMemoryView(reinterpret_cast<char*>(ptr), format_char<T>(), itemCount, readOnly) {
            static_assert(std::is_standard_layout<T>::value, "QPyArray requires standard layout type T");
        }
        QVariant opertaor () const {
            return QVariant::fromValue(static_cast<QPyMemoryView>(*this));
        }
        T* data() {
            return reinterpret_cast<T*>(data_u8());
        }
        const T* constData() const {
            return reinterpret_cast<const T*>(cdata_u8());
        }

        std::size_t size() const noexcept {
            return QPyMemoryView::size();
        }

        T operator [](int idx) {
            return data()[idx];
        }

        T at(int idx) {
            if (idx < 0 || static_cast<std::size_t>(idx) >= size()) {
                throw std::out_of_range("QPyArray::at: index out of range");
            }
            return data()[idx];
        }

    };
} // qtpyt

