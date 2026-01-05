#pragma once

#include <QMainWindow>
#include <QTimer>

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
    void resetTimer();

private:
    QTimer m_timer;
    int m_seconds = 0;

    QLabel *m_label;
    QPushButton *m_startBtn;
    QPushButton *m_stopBtn;
    QPushButton *m_resetBtn;

    void updateLabel();
};
