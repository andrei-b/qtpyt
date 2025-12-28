#include "q_py_execute_event.h"

void QPyExecuteEvent::execute() {
    if (!m_callable)
        return;
    try {
        //m_callable->run();
    } catch (const std::exception& e) {
        qWarning() << "QPyExecuteEvent: exception:" << e.what();
    } catch (...) {
        qWarning() << "QPyExecuteEvent: unknown exception";
    }
}