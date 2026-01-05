#pragma once
#include <QObject>
#include <qtpyt/qpymodule.h>


class PythonRunner : QObject {
    Q_OBJECT
public:
    explicit PythonRunner(const QString& scriptPath, QObject* parent = nullptr);
public slots:
    void runFunction();
private:
    qtpyt::QPyModule m_module;
};


