#pragma once

#define PYBIND11_NO_KEYWORDS
#include <pybind11/pybind11.h>
#include <QObject>
#include <utility>
#include <qtpyt/qpyfuture.h>
#include <qtpyt/qpymodule.h>

class QPyFutureImpl {
  public:
    virtual ~QPyFutureImpl();
    QPyFutureImpl(const qtpyt::QPyModule& module, QSharedPointer<qtpyt::IQPyFutureNotifier>&& notifier, QString  functionName, QByteArray  returnType, QVariantList&& arguments);
    QPyFutureImpl(const qtpyt::QPyModule& module, QSharedPointer<qtpyt::IQPyFutureNotifier>&& notifier, QString  functionName, QByteArray  returnType, const QVector<int>& types, void **a);
    void run();

    int resultCount() const;
    QVariant resultAsVariant(int index) const;

     qtpyt::QPyFutureState state() const {
        return m_state;
    }

    qtpyt::QPyModule * modulePtr() {
        return &m_module;
    }
    QString errorMessage() const;

  private:
    void pushResult(QVariant result);
    QByteArray m_returnType;
    mutable std::mutex m_mutex;
    qtpyt::QPyModule m_module;
    QString m_functionName;
    QVariantList m_arguments;
    QVariantList m_result;
     qtpyt::QPyFutureState m_state{ qtpyt::QPyFutureState::NotStarted};
    QSharedPointer<qtpyt::IQPyFutureNotifier> m_notifier{nullptr};
    QString m_errorMessage{};
};
