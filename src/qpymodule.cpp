#include <qtpyt/qpymodule.h>

#include <qtpyt/q_py_thread_pool.h>

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

    QPyModule::QPyModule(const QString& source, QPySourceType sourceType, const QString& funcName)
        : QPyModuleBase(source, sourceType, funcName) {
        addWantsToCancel();
    }

    QPyModule::~QPyModule() {}


    std::optional<QPyFuture> QPyModule::callAsync(const QString& functionName, QVariantList&& args) {
        const auto self = shared_from_this();
        if (functionName.isEmpty()) {
            return std::nullopt;
        }
        QPyFuture me(self, functionName, std::move(args));
        QPyThreadPool::instance().submit(me);
        return me;
    }

    /*void QPyModule::callInThread(QPyThread* thread) {
            const auto self = shared_from_this();
            thread->postExecute(self);
    }*/

    auto QPyModule::getThreadId() const {
        return m_threadId;
    }

    void QPyModule::addWantsToCancel() {
        py::gil_scoped_acquire gil;
        getPyCallable().attr("wantsToCancel") = py::cpp_function([this]() -> bool { return m_wantsToCancel; });
        getPyCallable().attr("setCancelReason") = py::cpp_function([this](const std::string& reason) {
            m_cancelReason = QString::fromStdString(reason);
            m_cancelled = true;
        });
    }

    void QPyModule::setThreadId() {
        //m_threadId = PyThreadState_Get()->thread_id;
    }
} // namespace qtpyt