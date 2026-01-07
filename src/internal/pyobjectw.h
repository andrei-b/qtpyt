#pragma once
#include <pybind11/pybind11.h>

namespace qtpyt {
    struct PyObjectWrapper {
        pybind11::object obj;
        PyObjectWrapper(pybind11::object obj) {
            this->obj = obj;
        }
    };
}