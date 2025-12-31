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
    QPyFutureImpl(QSharedPointer<qtpyt::QPyModule> module, QSharedPointer<qtpyt::QPyFutureNotifier>&& notifier, QString  functionName, QByteArray  returnType, QVariantList&& arguments);
    QPyFutureImpl(QSharedPointer<qtpyt::QPyModule> module, QSharedPointer<qtpyt::QPyFutureNotifier>&& notifier, QString  functionName, QByteArray  returnType, const QVector<int>& types, void **a);
    void run();

    int resultCount() const;

    QVariant resultAsVariant(int index) const;

     qtpyt::QPyFutureState state() const {
        return m_state;
    }

    qtpyt::QPyModule * callablePtr() const {
        return m_module.get();
    }
    QString errorMessage() const;

  private:
    void pushResult(QVariant result);
    QByteArray m_returnType;
    mutable std::mutex m_mutex;
    QSharedPointer<qtpyt::QPyModule> m_module;
    QString m_functionName;
    pybind11::tuple m_arguments;
    QVariantList m_result;
     qtpyt::QPyFutureState m_state{ qtpyt::QPyFutureState::NotStarted};
    QSharedPointer<qtpyt::QPyFutureNotifier> m_notifier{nullptr};
    QString m_errorMessage{};
};
