#pragma once
#define PYBIND11_NO_KEYWORDS

#include <qtpyt/qpymodule.h>

#include <QDebug>
#include <QEvent>
#include <memory>

namespace qtpyt { class QPyModule; }  // forward-declare the callable type

class QPyExecuteEvent : public QEvent {
public:
    explicit QPyExecuteEvent(std::shared_ptr<qtpyt::QPyModule> callable)
        : QEvent(static_cast<QEvent::Type>(eventType())), m_callable(std::move(callable)) {}



    static int eventType() {
        static int type = QEvent::registerEventType();
        return type;
    }

    // Call this from the receiver's event(...) to run the callable on the receiver thread.
    void execute();

    std::shared_ptr<qtpyt::QPyModule> callable() const { return m_callable; }

private:
    std::shared_ptr<qtpyt::QPyModule> m_callable;
};

