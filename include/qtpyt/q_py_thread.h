#pragma once

#include <QThread>
#include <QCoreApplication>
#include <QEvent>
#include <memory>

namespace qtpyt {
    class QPyExecuteEvent;
    class QPyModule;

    class QPyThread : public QThread {
    public:
        explicit QPyThread(QObject* parent = nullptr)
            : QThread(parent) {}

        ~QPyThread() override {
            quit();
            wait();
        }

        void run() override {
            exec();
        }
        bool event(QEvent* ev) override;
        void postExecute(std::shared_ptr<QPyModule> callable);
    };
} // namespace qtpyt