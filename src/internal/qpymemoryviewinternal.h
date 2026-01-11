#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace py = pybind11;

namespace qtpyt {
    static std::size_t itemsize_from_format(char c) {
        switch (c) {
            case 'b': return 1; case 'B': return 1;
            case 'h': return 2; case 'H': return 2;
            case 'i': return 4; case 'I': return 4;
            case 'q': return 8; case 'Q': return 8;
            case 'f': return 4;
            case 'd': return 8;
            case '?': return 1;
            default: throw std::invalid_argument(std::string("Unsupported format: ") + c);
        }
    }

    class QPyMemoryViewInternal {
    public:
        QPyMemoryViewInternal() = default;
        QPyMemoryViewInternal(char * ptr, char fmt, std::size_t count, bool readOnly);
        QPyMemoryViewInternal(const py::memoryview &mv);
        QPyMemoryViewInternal(const QPyMemoryViewInternal& other)= delete;
        QPyMemoryViewInternal& operator=(const QPyMemoryViewInternal& other)= delete;
        QPyMemoryViewInternal(QPyMemoryViewInternal&& other) noexcept = default;
        QPyMemoryViewInternal& operator=(QPyMemoryViewInternal&& other) = default;
        ~QPyMemoryViewInternal() = default;

        py::object memoryview() const;

        std::size_t nbytes() const noexcept;

        std::size_t size() const noexcept;

        std::size_t itemsize() const noexcept;

        char format() const noexcept { return m_fmt; }


        std::uint8_t* data_u8();

        const std::uint8_t* cdata_u8() const;

        void fill_byte(std::uint8_t v);

        // Copy in/out (handy for tests)
        void write_bytes(std::size_t offset, py::bytes src);

        py::bytes read_bytes(std::size_t offset, std::size_t len) const;

    private:
        char m_fmt;
        std::size_t m_itemsize;
        std::size_t m_count;
        std::size_t m_nbytes;
        void* m_ptr;
        py::object m_view;
    };


}// namespace qtpyt