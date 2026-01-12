#pragma once

#include <QMetaType>
#include <QObject>
#include <QVariant>
#include <memory>

#include "qpymodulebase.h"

class QPyFutureImpl;
namespace qtpyt {
    class QPyModule;
    enum class QPyFutureState { NotStarted, Running, Finished, Canceled, Error };

    class IQPyFutureNotifier  {
      public:
        virtual void notifyStarted(const QString& functionName) = 0;
        virtual void notifyFinished(const QString& functionName, const QVariant& value) = 0;
        virtual void notifyResultAvailable(const QString& functionName, const QVariant& value)  = 0;
        virtual void notifyErrorOccurred(const QString& functionName, const QString& errorMessage) = 0;
        virtual  ~IQPyFutureNotifier() = default;
    };

    class QPyFuture {
    public:
        QPyFuture(QPyModule module, QSharedPointer<IQPyFutureNotifier> notifier, const QString& functionName,  const QByteArray& returnType,
            QVariantList&& arguments);
        QPyFuture(QPyModule module, QSharedPointer<IQPyFutureNotifier> notifier, QString  functionName,  const QByteArray& returnType,
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

        [[nodiscard]] QPyModule* callablePtr() const;
        [[nodiscard]] QString errorMessage() const;

    private:

        std::shared_ptr<QPyFutureImpl> m_impl;
    };
} // namespace qtpyt