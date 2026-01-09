#pragma once
#include <functional>

namespace qtpyt {

    void registerSequenceReferenceType();

    class QPySequenceReference {
    public:
        QPySequenceReference() = default;
        explicit QPySequenceReference(std::function<void*()>&& func, char fmt, std::size_t itemSize,
            std::size_t itemCount, bool readOnly) : m_pointer(std::move(func)),
            m_fmt(fmt), m_itemSize(itemSize), m_itemCount(itemCount), m_readOnly(readOnly)
        {}
        virtual ~QPySequenceReference() = default;
        void setPointerFunction(std::function<void*()>&& func) {
            m_pointer = std::move(func);
        }
        void* getPointer() {
            if (m_pointer) {
                return m_pointer();
            }
            return nullptr;
        }
        char format() const noexcept {
            return m_fmt;
        }
        std::size_t itemSize() const noexcept {
            return m_itemSize;
        }
        std::size_t itemCount() const noexcept {
            return m_itemCount;
        }
        bool isReadOnly() const noexcept {
            return m_readOnly;
        }
    private:
        std::function<void*()> m_pointer;
        char m_fmt{0};
        std::size_t m_itemSize{0};
        std::size_t m_itemCount{0};
        bool m_readOnly{false};
    };

    template<typename T>
    auto __format_char = []( ) {
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
    };

    template <typename T>
    QPySequenceReference wrapVectorWithSequenceReference(const QSharedPointer<QVector<T>>& vecPtr, bool readOnly = false) {

        return QPySequenceReference(
            [vecPtr]() -> void* {
                return vecPtr->data();
            },
            __format_char<T>(),
            sizeof(T),
            vecPtr->length(),
            readOnly
        );
    }

    template <typename T>
    QPySequenceReference wrapVectorWithSequenceReference(QVector<T>* vecPtr, bool readOnly = false) {

        return QPySequenceReference(
            [vecPtr]() -> void* {
                return vecPtr;
            },
            __format_char<T>(),
            sizeof(T),
            vecPtr->length(),
            readOnly
        );
    }

    template <typename T>
    QPySequenceReference wrapVectorWithSequenceReference(const QSharedPointer<std::vector<T>>& vecPtr, bool readOnly = false) {

        return QPySequenceReference(
            [vecPtr]() -> void* {
                return vecPtr.get();
            },
            __format_char<T>(),
            sizeof(T),
            vecPtr->length(),
            readOnly
        );
    }

    template <typename T>
    QPySequenceReference wrapVectorWithSequenceReference(std::vector<T>* vecPtr, bool readOnly = false) {

        return QPySequenceReference(
            [vecPtr]() -> void* {
                return vecPtr;
            },
            __format_char<T>(),
            sizeof(T),
            vecPtr->length(),
            readOnly
        );
    }


} // qtpyt

