#include "q_py_future_impl.h"
#include "../q_embed_meta_object_py.h"
#include <qtpyt/conversions.h>

#include <utility>

QPyFutureImpl::~QPyFutureImpl() {
    pybind11::gil_scoped_acquire gil;
        m_arguments.release();// = py::none();
        m_result.release();// = py::none();

}
QPyFutureImpl::QPyFutureImpl(std::shared_ptr< qtpyt::QPyModule> callable, QString functionName, QVariantList&& arguments)
    : m_callable{std::move(callable)}, m_functionName(std::move(functionName)) {
    py::tuple argsTuple(arguments.size());
    for (int i = 0; i < arguments.size(); ++i) {
        py::object argObj =  qtpyt::qvariantToPyObject(arguments[i]);
        argsTuple[i] = argObj;
    }
    m_arguments = std::move(argsTuple);
}

QPyFutureImpl::QPyFutureImpl(std::shared_ptr< qtpyt::QPyModule> callable, QString  functionName,
                             const QVector<int>& types, void** a) : m_callable{std::move(callable)}, m_functionName(std::move(functionName)) {
    py::tuple argsTuple(types.size());
    for (int i = 0; i < types.size(); ++i) {
        const int typeId = types[i];
        argsTuple[i] =  qtpyt::qmetatypeToPyObject(typeId, a[i + 1]);
    }
    m_arguments = std::move(argsTuple);
}


void QPyFutureImpl::run() {
    try {
        pybind11::gil_scoped_acquire gil;
        {
            m_state =  qtpyt::QPyFutureState::Running;
            if (m_notifier.get() != nullptr) {
                m_notifier->notifyStarted();
            }
            // specify the return type explicitly and pass an explicit empty kwargs dict

            py::object result =
               pycall_internal__::call_python_no_kw(m_callable->makeCallable(m_functionName).value(), m_arguments);
            pushResult(std::move(result));
            m_state =  qtpyt::QPyFutureState::Finished;
            if (m_notifier.get() != nullptr) {
                m_notifier->notifyFinished();
            }
        }

    } catch (const py::error_already_set& e) {
        m_state =  qtpyt::QPyFutureState::Error;
        {
                std::lock_guard lock(m_mutex);
                m_errorMessage = QString::fromStdString(e.what());
        }
        if (m_notifier.get() != nullptr) {
            m_notifier->notifyErrorOccurred(QString::fromStdString(e.what()));
        }
        qWarning() << "Python error in QPyFutureImpl::run:" << e.what();
        // Handle Python exception (log it, store it, etc.)
    }

    catch (const std::exception& e) {
        m_state =  qtpyt::QPyFutureState::Error;
        {
                std::lock_guard lock(m_mutex);
                m_errorMessage = QString::fromStdString(e.what());
        }
        if (m_notifier.get() != nullptr) {
            m_notifier->notifyErrorOccurred(QString::fromStdString(e.what()));
        }
        qWarning() << e.what();
        // Handle exception (log it, store it, etc.)
    }
}

int QPyFutureImpl::resultCount() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_result.is(py::none())) {
        return 0;
    }
    return static_cast<int>(py::len(m_result));
}

QVariant QPyFutureImpl::resultAsVariant(const QString& resultType, int index) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_result.is(py::none())) {
        return QVariant();
    }
    if (index < 0 || index >= static_cast<int>(py::len(m_result))) {
        return QVariant();
    }
    py::object resultObj = m_result[index];
    auto v =  qtpyt::pyObjectToQVariant(resultObj, resultType.toUtf8());
        if (!v.has_value()) {
            qWarning() << "QPyFutureImpl::resultAsVariant: conversion to QVariant failed for result at index" << index;
                return QVariant();
        }
    return v.value();
}

std::shared_ptr< qtpyt::QPyFutureNotifier> QPyFutureImpl::connectNotifier() {
    m_notifier =  std::make_shared< qtpyt::QPyFutureNotifier>();
    return m_notifier;
}

QString QPyFutureImpl::errorMessage() const {
    std::lock_guard lock(m_mutex);
    return m_errorMessage;
}

void QPyFutureImpl::pushResult(py::object&& result) {
    std::lock_guard<std::mutex> lock(m_mutex);
    pybind11::gil_scoped_acquire gil;
    if (m_result.is(py::none())) {
        m_result = py::list();
    }

    if (result.is(py::none())) {
        // Python returned None -> treat as no results
        return;
    }

    m_result.append(std::move(result));

}
