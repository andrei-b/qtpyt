#pragma once
#define PYBIND11_NO_KEYWORDS
#include "qpymodulebase.h"
#include "qpyfuture.h"
#include "q_py_thread.h"
#include <QObject>
#include <QRunnable>
#include <pybind11/pybind11.h>

#include "qpythreadpool.h"

namespace qtpyt {
    class QPySlot;

    class QPyModule : public QPyModuleBase, public QEnableSharedFromThis<QPyModule> {
    public:
        static void makeQPyAsyncModule();

        static QSharedPointer<QPyModule> create(const QString &source, QPySourceType sourceType) {
            return QSharedPointer<QPyModule>(new QPyModule(source, sourceType));
        }

        QPyModule(const QString &source, QPySourceType sourceType);

        ~QPyModule() override;

        template<typename... Args>
        std::optional<QPyFuture> callAsync(const QSharedPointer<QPyFutureNotifier> &notifier,
                                           const QString &functionName, const QPyRegisteredType &returnType,
                                           Args... args) {
            QVariantList varArgs = {args...};
            return callAsync(notifier, functionName, returnType, std::move(varArgs));
        }

        std::optional<QPyFuture> callAsync(const QSharedPointer<QPyFutureNotifier> &notifier,
                                           const QString &functionName, const QPyRegisteredType &returnType,
                                           QVariantList
                                           &&args);

        template<typename R, typename... Args>
        std::function<std::optional<QPyFuture>(Args...)> makeAsyncFunction(
            const QSharedPointer<QPyFutureNotifier> &notifier, const QString &name) {
            auto self = sharedFromThis();
            const QMetaType returnType = QMetaType::fromType<R>();

            return [self, name, returnType, notifier](Args... args) -> std::optional<QPyFuture> {
                QVariantList varArgs;
                varArgs.reserve(sizeof...(Args));
                (varArgs.push_back(QVariant::fromValue(args)), ...);

                auto futOpt = self->callAsync(notifier, name, returnType, std::move(varArgs));
                if (!futOpt) {
                    throw std::runtime_error("callAsync failed");
                }
                return futOpt;
            };
        }

        QPySlot makeSlot(const QString &slotName, const QPyRegisteredType &returnType = QMetaType::Void,
                         const QSharedPointer<QPyFutureNotifier> &notifier = nullptr);

    protected:
        auto getThreadId() const;

        void setThreadId();

    private:
        void addWantsToCancel();

    private:
        QString m_cancelReason;
        bool m_cancelled{false};
        bool m_finished{false};
        bool m_wantsToCancel{false};
        qulonglong m_threadId{0};
    };
} // namespace qtpyt
