#define PYBIND11_NO_KEYWORDS
#include <pybind11/pybind11.h>
#include <qtpyt/q_py_thread.h>
#include "internal/q_py_execute_event.h"


namespace qtpyt {
    bool QPyThread::event(QEvent* ev) {
        if (ev && ev->type() == ::QPyExecuteEvent::eventType()) {
            // Safe to static_cast because we checked the event type.
            auto* exe = static_cast<::QPyExecuteEvent*>(ev);
            exe->execute();
            return true;
        }
        return QThread::event(ev);
    }
    void QPyThread::postExecute(std::shared_ptr<QPyModule> callable) {
        QCoreApplication::postEvent(this, new ::QPyExecuteEvent(callable));
    }
}