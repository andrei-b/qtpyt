#include <pybind11/pybind11.h>
#include <qtpyt/qpymodule.h>

#include <qtpyt/qpythreadpool.h>
#include <qtpyt/qpyslot.h>


namespace py = pybind11;

namespace qtpyt {
    static py::object g_CancelledExc;
    static py::module_ qpyasync_module;

    bool inject_cancelled(unsigned long target_tid) {
        if (!target_tid)
            return false;

        py::gil_scoped_acquire gil;

        if (!g_CancelledExc || g_CancelledExc.is_none())
            return false;

        PyObject* exc_type = g_CancelledExc.ptr(); // borrowed
        // PyThreadState_SetAsyncExc will INCREF internally? (not guaranteed / not documented as such)
        // So we ensure lifetime across the call explicitly:
        Py_INCREF(exc_type);

        const int rc = PyThreadState_SetAsyncExc(static_cast<unsigned long>(target_tid), exc_type);

        Py_DECREF(exc_type);

        if (rc == 1)
            return true;

        if (rc > 1) {
            PyThreadState_SetAsyncExc(static_cast<unsigned long>(target_tid), nullptr);
        }
        return false;
    }

    void QPyModule::makeQPyAsyncModule() {
        py::gil_scoped_acquire gil;
        qpyasync_module= py::module_::create_extension_module("qpyasync", "", nullptr);
        qpyasync_module.add_object("qpyasync.cancelled", g_CancelledExc, false);
    }

    QPyModule::QPyModule(const QString& source, QPySourceType sourceType)
        : QPyModuleBase(source, sourceType) {
        addWantsToCancel();
    }

    QPyModule::~QPyModule() {}


    std::optional<QPyFuture> QPyModule::callAsyncVariant(const QSharedPointer<IQPyFutureNotifier> &notifier, const QString& functionName, const QPyRegisteredType& returnType, QVariantList&& args) const {
        if (functionName.isEmpty()) {
            return std::nullopt;
        }
        QByteArray type = "void";
        if (std::holds_alternative<QMetaType>(returnType)) {
            type = std::get<QMetaType>(returnType).name();
        } else if (std::holds_alternative<QString>(returnType)) {
            type = std::get<QString>(returnType).toUtf8();
        } else if (std::holds_alternative<QMetaType::Type>(returnType)) {
            type = QMetaType(std::get<QMetaType::Type>(returnType)).name();
        }

        QPyFuture me(*this, notifier, functionName, type, std::move(args));
        QPyThreadPool::instance().submit(me);
        return me;
    }

    /*void QPyModule::callInThread(QPyThread* thread) {
            const auto self = shared_from_this();
            thread->postExecute(self);
    }*/

    QPySlot QPyModule::makeSlot(const QString &slotName, const QPyRegisteredType &returnType,
        const QSharedPointer<IQPyFutureNotifier> &notifier) {
        return QPySlot(*this, notifier, slotName, returnType);
    }

    QMetaObject::Connection QPyModule::connectToSignal(QObject *sender, const char *signal, const char *slot,
        Qt::ConnectionType type) const {
        return QPySlot::connectPythonFunction(sender, signal, *this, QString(slot), QMetaType::fromType<void>(), type);
    }

    QMetaObject::Connection QPyModule::connectAsyncToSignal(QObject *sender, const char *signal, const char *slot,
        Qt::ConnectionType type) const {
        return QPySlot::connectPythonFunctionAsync(sender, signal, *this, nullptr, QString(slot), QMetaType::fromType<void>());
    }

    auto QPyModule::getThreadId() const {
        return m_threadId;
    }

    void QPyModule::addWantsToCancel() {

    }

    void QPyModule::setThreadId() {
        //m_threadId = PyThreadState_Get()->thread_id;
    }
} // namespace qtpyt