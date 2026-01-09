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
} // qtpyt

