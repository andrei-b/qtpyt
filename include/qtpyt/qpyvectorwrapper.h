#pragma once
#include "qpyconvertiblepointer.h"
#include <QSharedPointer>
#include <QVariant>

namespace qtpyt {

    template <typename T>
    constexpr char format_char_of() {
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
    class QPyVectorWrapper {
    public:
        explicit QPyVectorWrapper(const QSharedPointer<QVector<T>>& vecPtr, bool readOnly = false) {
            m_ptr = QPyConvertiblePointer(
                [vecPtr]() -> void* {
                    return vecPtr->data();
                },
                format_char_of<T>(),
                sizeof(T),
                vecPtr->length(),
                readOnly
            );
        }
        explicit QPyVectorWrapper(const QSharedPointer<std::vector<T>>& vecPtr, bool readOnly = false) {
            m_ptr = QPyConvertiblePointer(
                [vecPtr]() -> void* {
                    return vecPtr->data();
                },
                format_char_of<T>(),
                sizeof(T),
                vecPtr->size(),
                readOnly
            );
        }

        QVariant operator()() const {
            return QVariant::fromValue(m_ptr);
        }

        QPyConvertiblePointer pointer() {
            return m_ptr;
        }
        virtual ~QPyVectorWrapper() = default;
    private:
        QPyConvertiblePointer m_ptr;
    };
} // qtoyt