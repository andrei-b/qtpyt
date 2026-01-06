#pragma once
#include <QObject>
#include <qtpyt/qpymodule.h>


class PythonRunner : public QObject {
    Q_OBJECT
public:
    explicit PythonRunner(const QString& scriptPath, QObject* parent = nullptr);
signals:
    void runFunction();
private:
    qtpyt::QPyModule m_module;
};


