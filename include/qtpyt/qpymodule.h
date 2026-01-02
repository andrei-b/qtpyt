/// \file qpymodule.h
/// \brief Public C\+\+ interface for invoking Python module functions via qtpyt.
/// \details
/// `QPyModule` represents a Python "module" (from source text or file) that can be
/// executed and interacted with from C\+\+ using Qt types. It supports synchronous
/// module setup (done in the base) and asynchronous function invocation returning
/// `QPyFuture`.
///
/// Threading model:\n
/// \- Instances track the creating thread id and expect interactions to respect
///   the underlying Qt/Python thread constraints.\n
/// \- Asynchronous calls are scheduled via qtpyt thread facilities (see `qpythreadpool.h`).
///
/// \note `PYBIND11_NO_KEYWORDS` is defined to avoid keyword argument collisions in pybind11.

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

/// \class QPyModule
/// \brief A Qt\-friendly wrapper around a Python module with async invocation support.
/// \details
/// `QPyModule` is constructed from Python source (text/file/etc. depending on
/// `QPySourceType`) and allows invoking named functions asynchronously.
/// The async API returns `std::optional<QPyFuture>` to indicate scheduling failure.
///
/// Lifetime:\n
/// The object is intended to be managed by `QSharedPointer` and supports
/// `sharedFromThis()` via `QEnableSharedFromThis`.
class QPyModule : public QPyModuleBase, public QEnableSharedFromThis<QPyModule> {
public:
    /// \brief Creates/initializes the Python async module glue (pybind11 module).
    /// \details
    /// Registers/constructs bindings necessary for asynchronous execution.
    /// \note Call semantics depend on the embedding/binding layer configuration.
    static void makeQPyAsyncModule();

    /// \brief Factory that returns a reference\-counted `QPyModule`.
    /// \param source Python source (content or path) as indicated by \p sourceType.
    /// \param sourceType Specifies how \p source is interpreted.
    /// \return A `QSharedPointer` owning the newly created module instance.
    static QSharedPointer<QPyModule> create(const QString &source, QPySourceType sourceType) {
        return QSharedPointer<QPyModule>(new QPyModule(source, sourceType));
    }

    /// \brief Constructs a module wrapper from Python source.
    /// \param source Python source (content or path) as indicated by \p sourceType.
    /// \param sourceType Specifies how \p source is interpreted.
    explicit QPyModule(const QString &source, QPySourceType sourceType);

    /// \brief Destructor.
    /// \details Ensures resources owned by the module wrapper are released.
    ~QPyModule() override;

    /// \brief Asynchronously calls a Python function by name, with variadic arguments.
    /// \details
    /// \tparam Args Arguments.
    /// \param notifier Optional notifier used to observe completion/progress.
    /// \param functionName Name of the Python callable to invoke.
    /// \param returnType Expected return type information for marshalling.
    /// \param args Arguments forwarded and converted into a `QVariantList`.
    /// \return `QPyFuture` on successful scheduling; `std::nullopt` on failure.
    template<typename... Args>
    std::optional<QPyFuture> callAsync(const QSharedPointer<QPyFutureNotifier> &notifier,
                                       const QString &functionName,
                                       const QPyRegisteredType &returnType,
                                       Args... args) {
        QVariantList varArgs = {args...};
        return callAsync(notifier, functionName, returnType, std::move(varArgs));
    }

    /// \brief Asynchronously calls a Python function by name with explicit arguments.
    /// \param notifier Optional notifier used to observe completion/progress.
    /// \param functionName Name of the Python callable to invoke.
    /// \param returnType Expected return type information for marshalling.
    /// \param args Argument list; moved into the call to avoid copies.
    /// \return `QPyFuture` on successful scheduling; `std::nullopt` on failure.
    std::optional<QPyFuture> callAsync(const QSharedPointer<QPyFutureNotifier> &notifier,
                                       const QString &functionName,
                                       const QPyRegisteredType &returnType,
                                       QVariantList &&args);

    /// \brief Creates a typed async function wrapper bound to this module.
    /// \details
    /// Produces a callable that invokes the specified Python function asynchronously
    /// passing it the arguments provided at call time.
    /// \tparam R Expected C\+\+ return type (mapped to `QMetaType`).
    /// \tparam Args Argument types convertible to `QVariant` via `QVariant::fromValue`.
    /// \param notifier Optional notifier used to observe completion/progress.
    /// \param name Name of the Python callable to invoke.
    /// \return A `std::function` that returns an optional `QPyFuture`.
    /// \throws std::runtime_error If the underlying `callAsync()` returns `std::nullopt`.
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

    /// \brief Creates a `QPySlot` that allows to connect the Python function to Qt's signals.
    /// \param slotName Name of the slot/function to bind.
    /// \param returnType Expected return type; defaults to `void`.
    /// \param notifier Optional notifier used to observe completion/progress.
    /// \return A `QPySlot` bound to \p slotName.
    QPySlot makeSlot(const QString &slotName,
                     const QPyRegisteredType &returnType = QMetaType::Void,
                     const QSharedPointer<QPyFutureNotifier> &notifier = nullptr);

protected:
    /// \brief Returns the thread id associated with this module instance.
    /// \return An implementation\-defined thread id value.
    auto getThreadId() const;

    /// \brief Captures/sets the current thread id as the module's associated thread.
    void setThreadId();

private:
    /// \brief Marks the module as wanting to cancel outstanding work.
    /// \details Used internally to coordinate cancellation state.
    void addWantsToCancel();

private:
    /// \brief Human\-readable cancel reason, if cancellation is requested.
    QString m_cancelReason;

    /// \brief True if cancellation has been triggered.
    bool m_cancelled{false};

    /// \brief True when execution is complete.
    bool m_finished{false};

    /// \brief True when cancellation has been requested but not yet applied.
    bool m_wantsToCancel{false};

    /// \brief Thread id captured for thread\-affinity/validation logic.
    qulonglong m_threadId{0};
};

} // namespace qtpyt
