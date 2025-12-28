#pragma once
#define PYBIND11_NO_KEYWORDS

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <string>
#include <vector>
#include <QString>





namespace annotations_internal__ {

namespace py = pybind11;



QPyAnnotation parse_annotation(const py::handle &annotation, int depth = 0);

} // namespace annotations_internal__