#pragma once
#include <pybind11/pybind11.h>
#include <type_traits>
#include <complex>
#include <cstdint>
#include <string>

namespace py = pybind11;

// -----------------------------
// PEP 3118 format generation
// -----------------------------
namespace pep3118 {

constexpr const char* native_byte_order_prefix() {
#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && defined(__ORDER_BIG_ENDIAN__)
#  if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    return "<";
#  elif (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    return ">";
#  else
    return "=";
#  endif
#elif defined(_WIN32)
    return "<"; // Windows is little-endian on supported architectures
#else
    return "="; // safest fallback
#endif
}

template <typename T>
std::string integral_code() {
    static_assert(std::is_integral_v<T>, "integral_code<T> requires integral T");
    if constexpr (std::is_same_v<T, bool>) {
        return "?";
    } else if constexpr (sizeof(T) == 1) {
        return std::is_signed_v<T> ? "b" : "B";
    } else if constexpr (sizeof(T) == 2) {
        return std::is_signed_v<T> ? "h" : "H";
    } else if constexpr (sizeof(T) == 4) {
        return std::is_signed_v<T> ? "i" : "I";
    } else if constexpr (sizeof(T) == 8) {
        return std::is_signed_v<T> ? "q" : "Q";
    } else {
        static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8,
                      "Unsupported integer size");
        return "";
    }
}

template <typename T>
std::string float_code() {
    static_assert(std::is_floating_point_v<T>, "float_code<T> requires floating-point T");
    if constexpr (sizeof(T) == 4) return "f";
    if constexpr (sizeof(T) == 8) return "d";
    // long double is messy across platforms; PEP 3118 uses 'g' for long double.
    if constexpr (std::is_same_v<T, long double>) return "g";
    static_assert(sizeof(T) == 4 || sizeof(T) == 8 || std::is_same_v<T, long double>,
                  "Unsupported float size for PEP 3118 mapping");
    return "";
}

template <typename T>
std::string complex_code() {
    static_assert(std::is_same_v<T, std::complex<float>> ||
                  std::is_same_v<T, std::complex<double>> ||
                  std::is_same_v<T, std::complex<long double>>,
                  "Unsupported complex type for PEP 3118 mapping");

    if constexpr (std::is_same_v<T, std::complex<float>>)       return "Zf";
    if constexpr (std::is_same_v<T, std::complex<double>>)      return "Zd";
    if constexpr (std::is_same_v<T, std::complex<long double>>) return "Zg";
    return "";
}

template <typename T, typename Enable = void>
struct format {
    // Default: refuse unless trivially copyable AND user chooses to expose as bytes.
    static std::string code() {
        static_assert(std::is_void_v<Enable>,
                      "pep3118::format<T> not specialized for this type");
        return "";
    }
};

template <typename T>
struct format<T, std::enable_if_t<std::is_integral_v<T>>> {
    static std::string code() { return integral_code<T>(); }
};

// Floating specialization
template <typename T>
struct format<T, std::enable_if_t<std::is_floating_point_v<T>>> {
    static std::string code() { return float_code<T>(); }
};

template <typename T>
struct format<T, std::enable_if_t<
    std::is_same_v<T, std::complex<float>> ||
    std::is_same_v<T, std::complex<double>> ||
    std::is_same_v<T, std::complex<long double>>
>> {
    static std::string code() { return complex_code<T>(); }
};

template <typename T>
std::string full_format_string(bool include_byte_order = true) {
    std::string s;
    if (include_byte_order) s += native_byte_order_prefix();
    s += format<T>::code();
    return s;
}

} // namespace pep3118
