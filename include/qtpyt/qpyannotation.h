#pragma once
#define PYBIND11_NO_KEYWORDS
#include <pybind11/pybind11.h>
#include <QString>

struct QPyAnnotation {
    bool has_annotation = false;          // false if inspect._empty
    bool is_any = false;                  // annotation == object / Any
    QString base;                     // "int", "str", "Optional", "Union", "List", ...
    std::vector<QPyAnnotation> args;   // inner types for generics
    QString repr;                     // safe string representation for logging
};
