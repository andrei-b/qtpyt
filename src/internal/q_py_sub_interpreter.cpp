#include "q_py_sub_interpreter.h"

SubInterpreter::SubInterpreter() {
    gil_state_ = PyGILState_Ensure();
    tstate_ = Py_NewInterpreter();       // returns the new PyThreadState*
    saved_tstate_ = PyEval_SaveThread(); // releases GIL, returns current tstate
}

SubInterpreter::~SubInterpreter() {
    PyEval_RestoreThread(saved_tstate_);
    Py_EndInterpreter(tstate_); // clean up interpreter state
    // Release the GIL state obtained with PyGILState_Ensure()
    PyGILState_Release(gil_state_);
}

SubInterpreter::SubInterpreter(SubInterpreter&& o) noexcept
    : gil_state_(o.gil_state_), tstate_(o.tstate_), saved_tstate_(o.saved_tstate_) {
    o.gil_state_ = PyGILState_STATE(0);
    o.tstate_ = nullptr;
    o.saved_tstate_ = nullptr;
}

