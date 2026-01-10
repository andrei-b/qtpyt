#include "q_py_future_impl.h"
#include "../conversions.h"

#include <utility>

QPyFutureImpl::~QPyFutureImpl() {

}

QPyFutureImpl::QPyFutureImpl(const qtpyt::QPyModule& module, QSharedPointer<qtpyt::IQPyFutureNotifier>&& notifier, QString functionName, QByteArray  returnType, QVariantList&& arguments)
    : m_returnType(std::move(returnType)), m_module{module}, m_functionName(std::move(functionName)), m_notifier(std::move(notifier)) {
    for (auto& arg : arguments) {
        m_arguments.append(std::move(arg));
    }
}

QPyFutureImpl::QPyFutureImpl(const qtpyt::QPyModule& module, QSharedPointer<qtpyt::IQPyFutureNotifier>&& notifier, QString  functionName, QByteArray  returnType,
                             const QVector<int>& types, void** a) : m_returnType(std::move(returnType)), m_module{module},
                             m_functionName(std::move(functionName)), m_notifier(std::move(notifier)) {

    for (int i = 0; i < types.size(); ++i) {
        const int typeId = types.at(i);
        if (typeId == QMetaType::Void) {
            m_arguments.append(QVariant());
            continue;
        }
        QVariant argVar = QVariant(QMetaType(typeId), a[i+1]);
        m_arguments.append(argVar);
    }
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
               auto res = m_module.call(m_functionName, QMetaType::Void, m_arguments);
               if (!res.second.isEmpty()) {
                     throw std::runtime_error("QPyFutureImpl::run: " + res.second.toStdString());
               }
               m_state =  qtpyt::QPyFutureState::Finished;
               if (m_notifier != nullptr) {
                   m_notifier->notifyFinished({});
               }
               return;
            }


            auto result = m_module.call(m_functionName, QMetaType::fromName(m_returnType), m_arguments);
            if (!result.first.has_value()) {
                if (result.second.isEmpty()) {
                    throw std::runtime_error("QPyFuture: You are probably waiting for the result from a function that does not return anything");
                }
                throw std::runtime_error("QPyFuture: " + result.second.toStdString());
            }
            pushResult(result.first.value());
            m_state =  qtpyt::QPyFutureState::Finished;
            if (m_notifier != nullptr) {
                m_notifier->notifyFinished(result.first.value());
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
