#pragma once
#define PYBIND11_NO_KEYWORDS
#include "qpymodulebase.h"
#include "qpyfuture.h"
#include "q_py_thread.h"
#include <QObject>
#include <QRunnable>
#include <pybind11/pybind11.h>

namespace qtpyt {

class QPyModule : public QPyModuleBase, public std::enable_shared_from_this<QPyModule> {
  public:
    static void makeQPyAsyncModule();
    QPyModule(const QString& source, QPySourceType sourceType, const QString& funcName);
    ~QPyModule() override;

    template <typename... Args>
    std::optional<QPyFuture> callAsync(const QString& functionName, const QPyRegisteredType& returnType, Args... args) {
        QVariantList varArgs = {args...};
        return callAsync(functionName, returnType, std::move(varArgs));
    }
    std::optional<QPyFuture> callAsync(const QString &functionName, const QPyRegisteredType &returnType, QVariantList &&args);
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

}  // namespace qtpyt