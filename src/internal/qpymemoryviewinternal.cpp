//
// Created by andrei on 1/5/26.
//

#include "qpymemoryviewinternal.h"
#include "stringpool.h"

qtpyt::QPyMemoryViewInternal::QPyMemoryViewInternal(char *ptr, char fmt, std::size_t count, bool readOnly): m_fmt(fmt),
    m_itemsize(itemsize_from_format(fmt)),
    m_count(count),
    m_nbytes(m_itemsize * m_count) ,
    m_ptr(static_cast<void*>(ptr)){
    auto fmtprt = StringPool::instance().intern(std::string(1, m_fmt))->c_str();
    m_view = py::memoryview::from_buffer(
        m_ptr,
        itemsize_from_format(m_fmt),
        fmtprt,
        { static_cast<py::ssize_t>(m_count) },         // shape
        { static_cast<py::ssize_t>(m_itemsize) },
        readOnly
    );

}

qtpyt::QPyMemoryViewInternal::QPyMemoryViewInternal(const py::memoryview &mv) {
    if (!mv)
        throw std::invalid_argument("Invalid memoryview");

    py::buffer_info info = py::buffer(mv).request();

    if (info.ndim != 1)
        throw std::invalid_argument("Only 1D memoryviews are supported");

    m_fmt = info.format[0]; // assume single char format
    m_itemsize = static_cast<std::size_t>(info.itemsize);
    m_count = static_cast<std::size_t>(info.shape[0]);
    m_nbytes = m_itemsize * m_count;
    m_view = mv;
    m_ptr = info.ptr;
}

py::object qtpyt::QPyMemoryViewInternal::memoryview() const {
    return m_view;
}

std::size_t qtpyt::QPyMemoryViewInternal::nbytes() const noexcept { return m_nbytes; }

std::size_t qtpyt::QPyMemoryViewInternal::size() const noexcept { return m_count; }

std::size_t qtpyt::QPyMemoryViewInternal::itemsize() const noexcept { return m_itemsize; }

std::uint8_t * qtpyt::QPyMemoryViewInternal::data_u8() {
   return static_cast<std::uint8_t*>(m_ptr);
}

const std::uint8_t * qtpyt::QPyMemoryViewInternal::cdata_u8() const {
    return static_cast<const std::uint8_t*>(m_ptr);
}

void qtpyt::QPyMemoryViewInternal::fill_byte(std::uint8_t v) {
    std::memset(data_u8(), v, m_nbytes);
}

void qtpyt::QPyMemoryViewInternal::write_bytes(std::size_t offset, py::bytes src) {
    std::string s = src; // copies bytes into std::string
    if (offset > m_nbytes || s.size() > (m_nbytes - offset))
        throw std::out_of_range("write_bytes out of range");
    std::memcpy(data_u8() + offset, s.data(), s.size());
}

py::bytes qtpyt::QPyMemoryViewInternal::read_bytes(std::size_t offset, std::size_t len) const {
    if (offset > m_nbytes || len > (m_nbytes - offset))
        throw std::out_of_range("read_bytes out of range");
    return py::bytes(reinterpret_cast<const char*>(cdata_u8() + offset),
                     static_cast<py::ssize_t>(len));
}
