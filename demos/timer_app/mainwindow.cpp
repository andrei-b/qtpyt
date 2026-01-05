#include "mainwindow.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    auto *central = new QWidget(this);
    setCentralWidget(central);

    m_label = new QLabel("00:00", this);
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setStyleSheet("font-size: 32px;");

    m_startBtn = new QPushButton("Start", this);
    m_stopBtn  = new QPushButton("Stop", this);
    m_resetBtn = new QPushButton("Reset", this);

    auto *btnLayout = new QHBoxLayout;
    btnLayout->addWidget(m_startBtn);
    btnLayout->addWidget(m_stopBtn);
    btnLayout->addWidget(m_resetBtn);

    auto *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_label);
    mainLayout->addLayout(btnLayout);

    central->setLayout(mainLayout);

    m_timer.setInterval(1000);

    connect(&m_timer, &QTimer::timeout, this, &MainWindow::onTick);
    connect(m_startBtn, &QPushButton::clicked, this, &MainWindow::startTimer);
    connect(m_stopBtn,  &QPushButton::clicked, this, &MainWindow::stopTimer);
    connect(m_resetBtn, &QPushButton::clicked, this, &MainWindow::resetTimer);

    setWindowTitle("Simple Qt Timer");
    resize(240, 140);
}

void MainWindow::onTick()
{
    ++m_seconds;
    updateLabel();
}

void MainWindow::startTimer()
{
    if (!m_timer.isActive())
        m_timer.start();
}

void MainWindow::stopTimer()
{
    m_timer.stop();
}

void MainWindow::resetTimer()
{
    m_timer.stop();
    m_seconds = 0;
    updateLabel();
}

void MainWindow::updateLabel()
{
    int minutes = m_seconds / 60;
    int seconds = m_seconds % 60;

    m_label->setText(
        QString("%1:%2")
            .arg(minutes, 2, 10, QLatin1Char('0'))
            .arg(seconds, 2, 10, QLatin1Char('0'))
    );
}
