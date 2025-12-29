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
    QPyFutureImpl(std::shared_ptr< qtpyt::QPyModule> callable, QString  functionName, const QByteArray& returnType, QVariantList&& arguments);
    QPyFutureImpl(std::shared_ptr< qtpyt::QPyModule> callable,  QString  functionName, const QByteArray& returnType, const QVector<int>& types, void **a);
    void run();

    int resultCount() const;

    QVariant resultAsVariant(int index) const;

     qtpyt::QPyFutureState state() const {
        return m_state;
    }
    std::shared_ptr< qtpyt::QPyFutureNotifier> connectNotifier();
     qtpyt::QPyModule * callablePtr() const {
        return m_callable.get();
    }
    QString errorMessage() const;

  private:
    void pushResult(QVariant result);
    QByteArray m_returnType;
    mutable std::mutex m_mutex;
    std::shared_ptr< qtpyt::QPyModule> m_callable;
    QString m_functionName;
    pybind11::tuple m_arguments;
    QVariantList m_result;
     qtpyt::QPyFutureState m_state{ qtpyt::QPyFutureState::NotStarted};
    std::shared_ptr< qtpyt::QPyFutureNotifier> m_notifier{nullptr};
    QString m_errorMessage{};
};
