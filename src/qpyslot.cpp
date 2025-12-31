#define  PYBIND11_NO_KEYWORDS
#include <pybind11/pybind11.h>

#include "qpyslot.h"

#include <qtpyt/conversions.h>
#include <qtpyt/qpythreadpool.h>
#include <QMetaMethod>
#include <utility>

namespace qtpyt {
    namespace {

        template <typename CallableType, typename AdditionalType>
        class QPySlotInternal : public QtPrivate::QSlotObjectBase {
        public:
            QPySlotInternal(QSharedPointer<CallableType> callable, QSharedPointer<QPyFutureNotifier> notifier, QMetaMethod method, void (*fn)(int which, QtPrivate::QSlotObjectBase* this_, QObject *receiver, void **args, bool *ret), AdditionalType *additional = nullptr ) :
            QSlotObjectBase(fn), m_callable(std::move(callable)), m_notifier(std::move(notifier)), m_method(method), m_additional(additional) {
                m_functionName = m_callable->functionName();
            };
            pybind11::object makeArgsTuple(void **a) const {
                const int n = m_method.parameterCount();
                py::tuple argsTuple(n);
                for (int i = 0; i < n; ++i) {
                    const int paramTypeId = m_method.parameterType(i);
                    if (paramTypeId == QMetaType::Void) {
                        argsTuple[i] = py::none();
                        continue;
                    }
                    py::object argObj = qmetatypeToPyObject(paramTypeId, a[i+1]);
                    argsTuple[i] = argObj;
                }
                return argsTuple;
            }

            auto callable()  {
                if (!m_callable) {
                    qWarning() << "QPySlot: callable is null for function" << m_functionName;
                }
                if (m_callable->functionName() != m_functionName) {
                    m_callable->setCallableFunction(m_functionName);
                }
                return m_callable;
            }

            const QMetaMethod& method() const {
                return m_method;
            }

            QSharedPointer<QPyFutureNotifier> notifier() const {
                return m_notifier;
            }

            AdditionalType * additional() const {
                return m_additional;
            }

            QString functionName() const {
                return m_functionName;
            }

            void makeParameterTypes() {
                const int n = m_method.parameterCount();
                QVector<int> types;
                types.reserve(n);
                for (int i = 0; i < n; ++i) {
                    const int paramTypeId = m_method.parameterType(i);
                    types.append(paramTypeId);
                }
                m_parameterTypes = types;
            }

            [[nodiscard]] const QVector<int> &parameterTypes() {
                if (m_parameterTypes.isEmpty()) {
                    makeParameterTypes();
                }
                return m_parameterTypes;
            }

        private:
            QVector<int> m_parameterTypes;
            QSharedPointer<CallableType> m_callable;
            QMetaMethod m_method;
            QString m_functionName;
            QSharedPointer<QPyFutureNotifier> m_notifier;
            AdditionalType * m_additional;
        };

    } // namespace

    void callProc(int which, QtPrivate::QSlotObjectBase* this_, QObject* r, void** a, bool* ret)  {
        if (which == 0) {
            return;
        }
        try {
            auto* slot = static_cast<QPySlotInternal<QPyModuleBase, void>*>(this_);
            const auto argsTuple = slot->makeArgsTuple(a);
            slot->callable()->call(argsTuple, {});
        } catch (const pybind11::error_already_set& e) {
            qWarning() << "Error in slot callable:" << e.what();
        }
    };

     QMetaObject::Connection QPySlot::connectCallable(QObject* sender, const char* signal, const QSharedPointer<QPyModuleBase> callable, Qt::ConnectionType type) {
        const auto pyCallableInfo = callable->inspectCallable();
        const auto signalMethod = findMatchingSignal(sender, signal, pyCallableInfo);
        if (!signalMethod) {
            qWarning() << "QPySlot::connectCallable: no matching signal found for" << signal;
            return {};
        }
        const auto mmethod = signalMethod.value();
        auto* slotObject = new QPySlotInternal<QPyModuleBase, void>(callable, nullptr, mmethod, callProc);
        const int signalIndex = mmethod.methodIndex();
        return QObjectPrivate::connect(sender, signalIndex, slotObject, type);
    }

    void callInThreadPool(int which, QtPrivate::QSlotObjectBase* this_, QObject* r, void** a, bool* ret)  {
        if (which == 0) {
            this_->destroyIfLastRef();
            return;
        }
        try {
            auto* slot = static_cast<QPySlotInternal<QPyModule, void>*>(this_);
            auto asyncCallable = slot->callable();

            if (asyncCallable.get()) {
                QPyFuture f = QPyFuture(asyncCallable, slot->notifier(), slot->functionName(), QByteArray("void"), slot->parameterTypes(),  a);
                QPyThreadPool::instance().submit(std::move(f));
            } else {
                qWarning() << "QPySlot::connectCallableAsync: callable is not a QPyModule";
            }
            this_->destroyIfLastRef();
        } catch (const pybind11::error_already_set& e) {
            qWarning() << "Error in async slot callable:" << e.what();
        }
    };

    QMetaObject::Connection QPySlot::connectCallableAsync(QObject* sender, const char* signal, QSharedPointer<QPyModule> callable, QSharedPointer<QPyFutureNotifier> notifier) {
        const auto pyCallableInfo = callable->inspectCallable();
        const auto signalMethod = findMatchingSignal(sender, signal, pyCallableInfo);
        if (!signalMethod) {
            qWarning() << "QPySlot::connectCallableAsync: no matching signal found for" << signal;
            return {};
        }
        const auto mmethod = signalMethod.value();
        //auto callableFunc = []
        auto* slotObject = new QPySlotInternal<QPyModule, void>(std::move(callable), std::move(notifier), mmethod, callInThreadPool);
        const int signalIndex = mmethod.methodIndex();
        return QObjectPrivate::connect(sender, signalIndex, slotObject, Qt::DirectConnection);
    }

    void callInThread(int which, QtPrivate::QSlotObjectBase* this_, QObject* r, void** a, bool* ret)  {
        if (which == 0) {
            return;
        }
        try {
            auto* slot = static_cast<QPySlotInternal<QPyModule, QPyThread>*>(this_);
            const auto argsTuple = slot->makeArgsTuple(a);

            if (auto asyncCallable = slot->callable(); auto thread = slot->additional()) {
                //asyncCallable->bindArguments(argsTuple);
                //asyncCallable->callInThread(thread);
            } else {
                qWarning() << "QPySlot::connectCallableAsync: callable is not a QPyModule";
            }
        } catch (const pybind11::error_already_set& e) {
            qWarning() << "Error in async slot callable:" << e.what();
        }
    };

    QMetaObject::Connection QPySlot::connectCallableAsync(QObject* sender, const char* signal,
        QSharedPointer<QPyModule> callable, QSharedPointer<QPyFutureNotifier> notifier, QPyThread* thread) {
        const auto pyCallableInfo = callable->inspectCallable();
        const auto signalMethod = findMatchingSignal(sender, signal, pyCallableInfo);
        if (!signalMethod) {
            qWarning() << "QPySlot::connectCallableAsync: no matching signal found for" << signal;
            return {};
        }
        const auto mmethod = signalMethod.value();
        //auto callableFunc = []
        auto* slotObject = new QPySlotInternal<QPyModule, QPyThread>(std::move(callable), std::move(notifier), mmethod, callInThread, thread);
        const int signalIndex = mmethod.methodIndex();
        return QObjectPrivate::connect(sender, signalIndex, slotObject, Qt::DirectConnection);
    }

    QMetaObject::Connection QPySlot::connectCallable(QObject* sender, int signalIndex, QSharedPointer<QPyModuleBase> module,
                                  QSharedPointer<QPyFutureNotifier> notifier, Qt::ConnectionType type) {
        auto mmethod = sender->metaObject()->method(signalIndex);
        auto* slotObject = new QPySlotInternal<QPyModuleBase, void>(std::move(module), notifier, mmethod, callProc);
        return QObjectPrivate::connect(sender, signalIndex, slotObject, type);
    }

    std::optional<QMetaMethod> QPySlot::findMatchingSignal(QObject* sender, const char* signal, const PyCallableInfo& pyCallableInfo) {
        const QMetaObject *mo = sender->metaObject();
        // Find method by name + arg count
        int methodIndex = -1;
        QByteArray qMethodName(signal);
        methodIndex = mo->indexOfSignal(QMetaObject::normalizedSignature(qPrintable(signal)));
        if (methodIndex >= 0) {
            if (QMetaMethod m = mo->method(methodIndex); m.parameterCount() != pyCallableInfo.arguments.size()) {
                methodIndex = -1; // arg count mismatch
            }
        }
        if (methodIndex < 0) {
            qWarning() << "QPySlot::findMatchingSignal: no matching signal " << signal << "num args: " << pyCallableInfo.arguments.size()
                    << "on " << mo->className();
            return {};
        }
        return mo->method(methodIndex);
    }
} // namespace qtpyt