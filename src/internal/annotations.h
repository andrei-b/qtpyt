#pragma once
#include <pybind11/pybind11.h>
#include <qtpyt/qpyannotation.h>




namespace annotations_internal__ {

namespace py = pybind11;



QPyAnnotation parse_annotation(const py::handle &annotation, int depth = 0);

} // namespace annotations_internal__