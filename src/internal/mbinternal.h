#pragma once

namespace qtpyt {
    struct MBInternal {
        /// \brief Stored reference to the currently selected callable object.
        pybind11::object callable;

        /// \brief Stored reference to the Python module object.
        pybind11::object m_module;
        ~MBInternal() {
            auto safe_release = [](pybind11::object &o) {
                if (!o || o.is_none()) return;

                if (!Py_IsInitialized() || PyGILState_GetThisThreadState() == nullptr) {
                    // Do not decref: can trip PyGILState_Check()/invalid tstate.
                    o.release();
                    return;
                }

                pybind11::gil_scoped_acquire gil;
                // Ensure decref happens while GIL is held.
                o = pybind11::none();
            };

            safe_release(callable);
            safe_release(m_module);

        }
    };
}