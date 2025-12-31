#include "q_py_future_impl.h"
#include "../q_embed_meta_object_py.h"
#include <qtpyt/conversions.h>

#include <utility>

QPyFutureImpl::~QPyFutureImpl() {
    if (!Py_IsInitialized()) {
        m_arguments.release();
        return;
    }

    if (PyGILState_GetThisThreadState() == nullptr) {
        m_arguments.release();
        return;
    }

    pybind11::gil_scoped_acquire gil;
    m_arguments.dec_ref();
}

QPyFutureImpl::QPyFutureImpl(QSharedPointer<qtpyt::QPyModule> module, QSharedPointer<qtpyt::QPyFutureNotifier>&& notifier, QString functionName, QByteArray  returnType, QVariantList&& arguments)
    : m_returnType(std::move(returnType)), m_module{std::move(module)}, m_functionName(std::move(functionName)), m_notifier(std::move(notifier)) {
    if (m_module == nullptr) {
        throw std::runtime_error("QPyFutureImpl: null module module");
    }
    pybind11::gil_scoped_acquire gil;
    py::tuple argsTuple(arguments.size());
    for (int i = 0; i < arguments.size(); ++i) {
        py::object argObj =  qtpyt::qvariantToPyObject(arguments[i]);
        argsTuple[i] = argObj;
    }
    m_arguments = std::move(argsTuple);
}

QPyFutureImpl::QPyFutureImpl(QSharedPointer<qtpyt::QPyModule> module, QSharedPointer<qtpyt::QPyFutureNotifier>&& notifier, QString  functionName, QByteArray  returnType,
                             const QVector<int>& types, void** a) : m_returnType(std::move(returnType)), m_module{std::move(module)},
                             m_functionName(std::move(functionName)), m_notifier(std::move(notifier)) {
    if (m_module == nullptr) {
        throw std::runtime_error("QPyFutureImpl: null module module");
    }
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
            if (m_notifier != nullptr) {
                m_notifier->notifyStarted();
            }
            // specify the return type explicitly and pass an explicit empty kwargs dict
            if (m_returnType == "void" || m_returnType == "NoneType") {
               pycall_internal__::call_python_no_kw(m_module->makeCallable(m_functionName).value(), m_arguments);
               m_state =  qtpyt::QPyFutureState::Finished;
               if (m_notifier != nullptr) {
                   m_notifier->notifyFinished();
               }
               return;
            }

            const py::object result =
               pycall_internal__::call_python_no_kw(m_module->makeCallable(m_functionName).value(), m_arguments);
            const auto var = qtpyt::pyObjectToQVariant(result, m_returnType);
            if (!var.has_value()) {
                throw std::runtime_error("QPyFutureImpl::run: conversion to QVariant failed for return type " + std::string(m_returnType));
            }
            pushResult(var.value());
            m_state =  qtpyt::QPyFutureState::Finished;
            if (m_notifier != nullptr) {
                m_notifier->notifyFinished(var.value());
            }
        }

    } catch (const py::error_already_set& e) {
        m_state =  qtpyt::QPyFutureState::Error;
        {
                std::lock_guard lock(m_mutex);
                m_errorMessage = QString::fromStdString(e.what());
        }
        if (m_notifier != nullptr) {
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
        if (m_notifier != nullptr) {
            m_notifier->notifyErrorOccurred(QString::fromStdString(e.what()));
        }
        qWarning() << e.what();
        // Handle exception (log it, store it, etc.)
    }
}

int QPyFutureImpl::resultCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return static_cast<int>(m_result.size());
}

QVariant QPyFutureImpl::resultAsVariant(int index) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (index < 0 || index >= m_result.size()) {
        qWarning() << "QPyFutureImpl::resultAsVariant: index out of range:" << index;
        return {};
    }
    return m_result.at(index);
}

QString QPyFutureImpl::errorMessage() const {
    std::lock_guard lock(m_mutex);
    return m_errorMessage;
}

void QPyFutureImpl::pushResult(QVariant result) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_result.append(std::move(result));

}
