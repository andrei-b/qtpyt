//
// Created by andrei on 1/5/26.
//

#include "pythonrunner.h"

PythonRunner::PythonRunner(const QString &scriptPath, QObject *parent) : QObject(parent),
    m_module(qtpyt::QPyModule(scriptPath, qtpyt::QPySourceType::File)) {
}

void PythonRunner::runFunction() {
    m_module.callAsync<>(nullptr,"mainFunction", QMetaType::Void);
}
