#pragma once

#define PYBIND11_NO_KEYWORDS
#include <pybind11/pybind11.h>
#include <QObject>
#include <utility>
#include <qtpyt/q_py_future.h>
#include <qtpyt/qpymodule.h>

class QPyFutureImpl {
  public:
    virtual ~QPyFutureImpl();
    QPyFutureImpl(std::shared_ptr< qtpyt::QPyModule> callable, QString  functionName, QVariantList&& arguments);
    QPyFutureImpl(std::shared_ptr< qtpyt::QPyModule> callable,  QString  functionName, const QVector<int>& types, void **a);
    void run();

    int resultCount();

    QVariant resultAsVariant(const QString& resultType, int index) const;

     qtpyt::QPyFutureState state() const {
        return m_state;
    }
    std::shared_ptr< qtpyt::QPyFutureNotifier> connectNotifier();
     qtpyt::QPyModule * callablePtr() const {
        return m_callable.get();
    }
    QString errorMessage() const;

  private:
    void pushResult(py::object&& result);
    mutable std::mutex m_mutex;
    std::shared_ptr< qtpyt::QPyModule> m_callable;
    QString m_functionName;
    pybind11::tuple m_arguments;
    py::list m_result;
     qtpyt::QPyFutureState m_state{ qtpyt::QPyFutureState::NotStarted};
    std::shared_ptr< qtpyt::QPyFutureNotifier> m_notifier{nullptr};
    QString m_errorMessage{};
};
