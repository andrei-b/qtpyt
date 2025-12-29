#pragma once
#define PYBIND11_NO_KEYWORDS
#include <pybind11/pybind11.h>
#include <QMetaType>
#include <QObject>
#include <QVariant>
#include <memory>

#include "qpymodulebase.h"

class QPyFutureImpl;
namespace qtpyt {
    class QPyModule;
    enum class QPyFutureState { NotStarted, Running, Finished, Canceled, Error };


    class QPyFutureNotifier : public QObject {
        Q_OBJECT
      public:
        QPyFutureNotifier() = default;
        ~QPyFutureNotifier() override = default;
        void notifyStarted() {
            emit started();
        }
        void notifyFinished() {
            emit finished();
        }
        void notifyErrorOccurred(const QString& errorMessage) {
            emit errorOccurred(errorMessage);
        }
        signals:
          void started();
        void finished();
        void errorOccurred(const QString& errorMessage);
    };


    class QPyFuture {
    public:
        QPyFuture(std::shared_ptr<QPyModule> callable, const QString& functionName,  const QByteArray& returnType,
            QVariantList&& arguments);
        QPyFuture(std::shared_ptr<QPyModule> callable, QString  functionName,  const QByteArray& returnType,
                                 const QVector<int>& types, void** a);
        QPyFuture(const QPyFuture& other);
        QPyFuture& operator=(const QPyFuture& other);

        void waitForFinished() const;

        QPyFuture(QPyFuture&& other) noexcept;
        QPyFuture& operator=(QPyFuture&& other);
        void operator()() const {
            this->run();
        }
        void run() const;
        [[nodiscard]] int resultCount() const;
        [[nodiscard]] QVariant resultAsVariant(int index) const;
        template <typename T> T resultAs(int index) {
            if (index < 0 || index >= this->resultCount()) {
                qWarning() << "QPyFuture::resultAs: index out of range:" << index;
                return T{};
            }
            const QVariant var = this->resultAsVariant(index);
            return var.value<T>();
        }
        [[nodiscard]] QPyFutureState state() const;
        [[nodiscard]] std::shared_ptr<QPyFutureNotifier> makeConnectNotifier() const;
        [[nodiscard]] QPyModule* callablePtr() const;
        [[nodiscard]] QString errorMessage() const;

    private:

        std::shared_ptr<QPyFutureImpl> m_impl;
    };
} // namespace qtpyt