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
#include "qpymodulebase.h"
#include "qpyfuture.h"
#include "q_py_thread.h"

#include <QObject>
#include <QRunnable>
#include "methodindex.h"

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
class QPyModule : public QPyModuleBase {
public:
    /// \brief Creates/initializes the Python async module glue (pybind11 module).
    /// \details
    /// Registers/constructs bindings necessary for asynchronous execution.
    /// \note Call semantics depend on the embedding/binding layer configuration.
    static void makeQPyAsyncModule();

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
    std::optional<QPyFuture> callAsync(const QSharedPointer<IQPyFutureNotifier> &notifier,
                                       const QString &functionName,
                                       const QPyRegisteredType &returnType,
                                       Args... args) const {
        QVariantList varArgs;
        (varArgs.push_back(QVariant::fromValue(args)), ...);
        return callAsyncVariant(notifier, functionName, returnType, std::move(varArgs));
    }

    /// \brief Asynchronously calls a Python function by name with explicit arguments.
    /// \param notifier Optional notifier used to observe completion/progress.
    /// \param functionName Name of the Python callable to invoke.
    /// \param returnType Expected return type information for marshalling.
    /// \param args Argument list; moved into the call to avoid copies.
    /// \return `QPyFuture` on successful scheduling; `std::nullopt` on failure.
    std::optional<QPyFuture> callAsyncVariant(const QSharedPointer<IQPyFutureNotifier> &notifier,
                                       const QString &functionName,
                                       const QPyRegisteredType &returnType,
                                       QVariantList &&args) const;


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
        const QSharedPointer<IQPyFutureNotifier> &notifier, const QString &name) {
        const QMetaType returnType = QMetaType::fromType<R>();
        return [module=*this, name, returnType, notifier](Args... args) -> std::optional<QPyFuture> {
            //QVariantList varArgs;
            //varArgs.reserve(sizeof...(Args));
            //(varArgs.push_back(QVariant::fromValue(args)), ...);

            auto futOpt = module.callAsync(notifier, name, returnType, args...);
            if (!futOpt) {
                throw std::runtime_error("callAsync failed");
            }
            return futOpt;
        };
    }

    /// \brief Creates a `QPySlot` that allows to connect the Python function to Qt's signals.
    /// Use the returned `QPySlot` to connect to signals if you need all the features of `QPySlot`
    /// (notifiers, processing return values) Otherwise use the simpler `connect(Async)ToSignal()` methods.
    /// \param slotName Name of the slot/function to bind.
    /// \param returnType Expected return type; defaults to `void`.
    /// \param notifier Optional notifier used to observe completion/progress.
    /// \return A `QPySlot` bound to \p slotName.
    QPySlot makeSlot(const QString &slotName,
                     const QPyRegisteredType &returnType = QMetaType::Void,
                     const QSharedPointer<IQPyFutureNotifier> &notifier = nullptr);

    /// \brief Connects a Qt signal to a Python function in this module (synchronous call).
    /// This is a convenience wrapper around `QPySlot::connectToSignal()`.
    /// \param sender The QObject emitting the signal.
    /// \param signal The signal signature (e.g., "signalName(int)").
    /// \param slot The name of the Python function to invoke when the signal is emitted.
    /// \param type The Qt connection type (default: Qt::AutoConnection).
    /// \return A QMetaObject::Connection representing the connection; invalid if connection fails.
    /// \note The Python function is called synchronously in the sender's thread context.
    QMetaObject::Connection connectToSignal(QObject* sender,
                                        const char* signal,
                                        const char* slot,
                                        Qt::ConnectionType type = Qt::AutoConnection) const;

    /// \brief Connects a Qt signal to a Python function in this module (asynchronous call).
    /// This is a convenience wrapper around `QPySlot::connectAsyncToSignal()` with notifier set to nullptr,
    /// and the slot return value discarded.
    /// \param sender The QObject emitting the signal.
    /// \param signal The signal signature (e.g., "signalName(int)").
    /// \param slot The name of the Python function to invoke when the signal is emitted.
    /// \param type The Qt connection type (default: Qt::AutoConnection).
    /// \return A QMetaObject::Connection representing the connection; invalid if connection fails.
    /// \note The Python function is scheduled asynchronously via qtpyt thread facilities.
    QMetaObject::Connection connectAsyncToSignal(QObject* sender,
                                        const char* signal,
                                        const char* slot,
                                        Qt::ConnectionType type = Qt::AutoConnection) const;

    /// \brief Connects a Qt signal to a Python function in this module (synchronous call, template version).
    /// \tparam SignalFunc The signal function pointer type.
    /// \param sender The QObject emitting the signal.
    /// \param signal The signal member function pointer.
    /// \param slot The name of the Python function to invoke when the signal is emitted.
    /// \param type The Qt connection type (default: Qt::AutoConnection).
    /// \return A QMetaObject::Connection representing the connection; invalid if connection fails.
    /// \note The Python function is called synchronously in the sender's thread context.
    /// \warning Logs a warning if the signal cannot be matched to a method signature.
    template <typename SignalFunc>
    QMetaObject::Connection connectToSignal(const typename QtPrivate::FunctionPointer<SignalFunc>::Object* sender,
        SignalFunc signal, const char* slot, Qt::ConnectionType type = Qt::AutoConnection) const {
        int signalIndex =  getMethodIndex<SignalFunc>(sender, signal).value_or(-1);
        if (signalIndex < 0) {
            qWarning("connectToSignal: cannot match the signal");
            return {};
        }
        const char* signalSignature = sender->metaObject()->method(signalIndex).methodSignature().constData();
        return connectToSignal(const_cast<QObject*>(static_cast<const QObject*>(sender)), signalSignature, slot, type);
    }

    /// \brief Connects a Qt signal to a Python function in this module (asynchronous call, template version).
    /// This is a convenience wrapper around `QPySlot::connectAsyncToSignal()` with notifier set to nullptr,
    /// and the slot return value discarded.
    /// \tparam SignalFunc The signal function pointer type.
    /// \param sender The QObject emitting the signal.
    /// \param signal The signal member function pointer.
    /// \param slot The name of the Python function to invoke when the signal is emitted.
    /// \param type The Qt connection type (default: Qt::AutoConnection).
    /// \return A QMetaObject::Connection representing the connection; invalid if connection fails.
    /// \note The Python function is scheduled asynchronously via qtpyt thread facilities.
    /// \warning Logs a warning if the signal cannot be matched to a method signature.
    template <typename SignalFunc>
    QMetaObject::Connection connectAsyncToSignal(const typename QtPrivate::FunctionPointer<SignalFunc>::Object* sender,
        SignalFunc signal, const char* slot, Qt::ConnectionType type = Qt::AutoConnection) const {
        int signalIndex =  getMethodIndex<SignalFunc>(sender, signal).value_or(-1);
        if (signalIndex < 0) {
            qWarning("connectToSignal: cannot match the signal");
            return {};
        }
        const char* signalSignature = sender->metaObject()->method(signalIndex).methodSignature().constData();
        return connectAsyncToSignal(const_cast<QObject*>(static_cast<const QObject*>(sender)), signalSignature, slot, type);
    }

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
