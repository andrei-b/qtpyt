#define  PYBIND11_NO_KEYWORDS
#include <pybind11/pybind11.h>

#include <qtpyt/qpyfuture.h>

#include "internal/q_py_future_impl.h"

namespace qtpyt {
    QPyFuture::QPyFuture(QSharedPointer<QPyModule> module, QSharedPointer<QPyFutureNotifier> notifier, const QString& functionName, const QByteArray& returnType,
                         QVariantList&& arguments) {
        m_impl = std::make_shared<QPyFutureImpl>(std::move(module), std::move(notifier), functionName, returnType, std::move(arguments));
    }

    QPyFuture::QPyFuture(QSharedPointer<QPyModule> module, QSharedPointer<QPyFutureNotifier> notifier, QString functionName, const QByteArray& returnType,
        const QVector<int>& types,  void** a) {
        m_impl = std::make_shared<::QPyFutureImpl>(std::move(module), std::move(notifier), std::move(functionName), returnType, types, a);
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

    QVariant QPyFuture::resultAsVariant(int index) const {
        return m_impl->resultAsVariant(index);
    }

    QPyFutureState QPyFuture::state() const {
        return m_impl->state();
    }

    QPyModule* QPyFuture::callablePtr() const {
        return m_impl->callablePtr();
    }

    QString QPyFuture::errorMessage() const {
        return m_impl->errorMessage();
    }
} // namespace qtpyt


