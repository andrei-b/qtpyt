#pragma once

#include <QMainWindow>
#include <QTimer>
#include <QSpinBox>
#include "pythonrunner.h"

class QLabel;
class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onTick();
    void startTimer();
    void stopTimer();
signals:
    void elapsed();
private:
    QTimer m_timer;
    int m_seconds = 0;

    QSpinBox *m_spin_box_;
    QPushButton *m_startBtn;
    QPushButton *m_stopBtn;
    PythonRunner* m_pythonRunner{new PythonRunner(QStringLiteral(":/slot.py"), this)};
};
