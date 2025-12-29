#define  PYBIND11_NO_KEYWORDS
#include <pybind11/pybind11.h>

#include <qtpyt/q_py_future.h>

#include "internal/q_py_future_impl.h"

namespace qtpyt {
    QPyFuture::QPyFuture(std::shared_ptr<QPyModule> callable, const QString& functionName,
                         QVariantList&& arguments) {
        m_impl = std::make_shared<QPyFutureImpl>(std::move(callable), functionName,std::move(arguments));
    }

    QPyFuture::QPyFuture(std::shared_ptr<QPyModule> callable, QString functionName, const QVector<int>& types,
                         void** a) {
        m_impl = std::make_shared<::QPyFutureImpl>(std::move(callable), std::move(functionName), types, a);
    }

    QPyFuture::QPyFuture(const QPyFuture& other) {
        m_impl = other.m_impl;
    }

    QPyFuture& QPyFuture::operator=(const QPyFuture& other) {
        if (this != &other) {
            m_impl = other.m_impl;
        }
        return *this;
    }

    void QPyFuture::waitForFinished() const {
        while (true) {
            const QPyFutureState s = this->state();
            if (s == QPyFutureState::Finished || s == QPyFutureState::Error || s == QPyFutureState::Canceled) {
                break;
            }
            QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
            QThread::msleep(1);
        }
    }

    QPyFuture::QPyFuture(QPyFuture&& other)  noexcept {
        m_impl = std::move(other.m_impl);
    }

    QPyFuture& QPyFuture::operator=(QPyFuture&& other) {
        if (this != &other) {
            m_impl = std::move(other.m_impl);
        }
        return *this;

    }

    int QPyFuture::resultCount() const {
        return m_impl->resultCount();
    }
    void QPyFuture::run() const {
        m_impl->run();
    }

    QVariant QPyFuture::resultAsVariant(const QPyRegisteredType& rt , int index) const {
        if (std::holds_alternative<QMetaType>(rt)) {
            return m_impl->resultAsVariant(std::get<QMetaType>(rt).name(), index);
        }
        if (std::holds_alternative<QMetaType::Type>(rt)) {
            return m_impl->resultAsVariant(QMetaType(std::get<QMetaType::Type>(rt)).name(), index);
        }
        if (std::holds_alternative<QString>(rt)) {
            return m_impl->resultAsVariant(std::get<QString>(rt), index);
        }
        return {};
    }

    QPyFutureState QPyFuture::state() const {
        return m_impl->state();
    }

    std::shared_ptr<QPyFutureNotifier> QPyFuture::makeConnectNotifier() const {
        return m_impl->connectNotifier();
    }

    QPyModule* QPyFuture::callablePtr() const {
        return m_impl->callablePtr();
    }

    QString QPyFuture::errorMessage() const {
        return m_impl->errorMessage();
    }
} // namespace qtpyt


