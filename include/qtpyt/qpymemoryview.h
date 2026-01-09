#pragma once
#include <cstddef>
#include <cstdint>
#include <QSharedPointer>

namespace qtpyt {

    int registerMemoryViewType();

    class QPyMemoryViewInternal;
    class QPyMemoryView {
    public:
        static int registerMemoryViewType();

        QPyMemoryView(char * ptr, char fmt, std::size_t itemCount, bool readOnly);
        QPyMemoryView() = default;
        std::size_t nbytes() const noexcept;

        std::size_t size() const noexcept;

        std::size_t itemsize() const noexcept;

        char format();

        std::uint8_t* data_u8();

        const std::uint8_t* cdata_u8() const;

        void fill_byte(std::uint8_t v);

        void writeData(std::size_t offset, const void* src, std::size_t len);

        void readData(std::size_t offset, const void* dst, std::size_t len);
    private:
        friend void setData(QSharedPointer<QPyMemoryViewInternal> d, QPyMemoryView& mv);
        friend QSharedPointer<QPyMemoryViewInternal> getData(const QPyMemoryView& mv);
        QSharedPointer<QPyMemoryViewInternal> m_data;

    };

} // qtpyt
