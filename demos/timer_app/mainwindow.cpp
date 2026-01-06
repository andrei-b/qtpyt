#include "mainwindow.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTime>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    auto *central = new QWidget(this);
    setCentralWidget(central);

    m_spin_box_ = new QSpinBox(this);
    m_spin_box_->setRange(0, 3600);
    m_spin_box_->setValue(5);

    m_spin_box_->setAlignment(Qt::AlignCenter);
    m_spin_box_->setStyleSheet("font-size: 32px;");

    m_startBtn = new QPushButton("Start", this);
    m_stopBtn  = new QPushButton("Stop", this);

    auto *btnLayout = new QHBoxLayout;
    btnLayout->addWidget(m_startBtn);
    btnLayout->addWidget(m_stopBtn);

    auto *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_spin_box_);
    mainLayout->addLayout(btnLayout);

    central->setLayout(mainLayout);

    m_timer.setInterval(1000);

    connect(&m_timer, &QTimer::timeout, this, &MainWindow::onTick);
    connect(m_startBtn, &QPushButton::clicked, this, &MainWindow::startTimer);
    connect(m_stopBtn,  &QPushButton::clicked, this, &MainWindow::stopTimer);

    setWindowTitle("QtPyT Timer");
    resize(200, 140);

    connect(this, &MainWindow::elapsed, m_pythonRunner, &PythonRunner::runFunction);
}

void MainWindow::onTick()
{
    m_seconds = m_spin_box_->value();
    m_seconds--;
    m_spin_box_->setValue(m_seconds);
    if (m_seconds == 0) {
        stopTimer();
        emit elapsed();
    }
}

void MainWindow::startTimer()
{

    if (!m_timer.isActive()) {
        m_timer.start();
    }
}

void MainWindow::stopTimer()
{
    m_timer.stop();
}