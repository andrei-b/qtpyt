//
// Created by andrei on 1/5/26.
//

#include "qpymemoryviewinternal.h"

qtpyt::QPyMemoryViewInternal::QPyMemoryViewInternal(char *ptr, char fmt, std::size_t count, bool readOnly): fmt_(fmt),
    itemsize_(itemsize_from_format(fmt)),
    count_(count),
    nbytes_(itemsize_ * count_) {
    backing_ = py::bytearray(ptr, nbytes_);
    py::buffer_info info = py::buffer(backing_).request();
    view_ = py::memoryview::from_buffer(
        info.ptr,
        itemsize_from_format(fmt_),
        std::string(1, fmt_).c_str(),
        { static_cast<py::ssize_t>(count_) },         // shape
        { static_cast<py::ssize_t>(itemsize_) },
        readOnly
    );

}

qtpyt::QPyMemoryViewInternal::QPyMemoryViewInternal(const py::memoryview &mv) {
    if (!mv)
        throw std::invalid_argument("Invalid memoryview");

    py::buffer_info info = py::buffer(mv).request();

    if (info.ndim != 1)
        throw std::invalid_argument("Only 1D memoryviews are supported");

    fmt_ = info.format[0]; // assume single char format
    itemsize_ = static_cast<std::size_t>(info.itemsize);
    count_ = static_cast<std::size_t>(info.shape[0]);
    nbytes_ = itemsize_ * count_;

    backing_ = py::bytearray(static_cast<char*>(info.ptr), nbytes_);
    view_ = mv;


}

py::object qtpyt::QPyMemoryViewInternal::memoryview() const {
    return view_;
}

py::bytearray qtpyt::QPyMemoryViewInternal::backing() const { return backing_; }

std::size_t qtpyt::QPyMemoryViewInternal::nbytes() const noexcept { return nbytes_; }

std::size_t qtpyt::QPyMemoryViewInternal::size() const noexcept { return count_; }

std::size_t qtpyt::QPyMemoryViewInternal::itemsize() const noexcept { return itemsize_; }

std::uint8_t * qtpyt::QPyMemoryViewInternal::data_u8() {
    const py::buffer_info info = py::buffer(backing_).request();
    return static_cast<std::uint8_t*>(info.ptr);
}

const std::uint8_t * qtpyt::QPyMemoryViewInternal::cdata_u8() const {
    const py::buffer_info info = py::buffer(backing_).request();
    return static_cast<const std::uint8_t*>(info.ptr);
}

void qtpyt::QPyMemoryViewInternal::fill_byte(std::uint8_t v) {
    std::memset(data_u8(), v, nbytes_);
}

void qtpyt::QPyMemoryViewInternal::write_bytes(std::size_t offset, py::bytes src) {
    std::string s = src; // copies bytes into std::string
    if (offset > nbytes_ || s.size() > (nbytes_ - offset))
        throw std::out_of_range("write_bytes out of range");
    std::memcpy(data_u8() + offset, s.data(), s.size());
}

py::bytes qtpyt::QPyMemoryViewInternal::read_bytes(std::size_t offset, std::size_t len) const {
    if (offset > nbytes_ || len > (nbytes_ - offset))
        throw std::out_of_range("read_bytes out of range");
    return py::bytes(reinterpret_cast<const char*>(cdata_u8() + offset),
                     static_cast<py::ssize_t>(len));
}
