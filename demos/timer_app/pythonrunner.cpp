//
// Created by andrei on 1/5/26.
//

#include "pythonrunner.h"

#include "qtpyt/qpyslot.h"

PythonRunner::PythonRunner(const QString &scriptPath, QObject *parent) : QObject(parent),
                                                                         m_module(qtpyt::QPyModule(scriptPath, qtpyt::QPySourceType::File)) {
    m_module.addVariable("main_window", parent);
    m_module.makeSlot("on_timer_elapsed", QMetaType::Void).connectAsyncToSignal(this, &PythonRunner::runFunction);
}

