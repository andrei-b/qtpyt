#define  PYBIND11_NO_KEYWORDS
#include <pybind11/pybind11.h>

#include "../include/qtpyt/qpyslot.h"

#include <qtpyt/conversions.h>
#include <qtpyt/qpythreadpool.h>
#include <QMetaMethod>
#include <utility>

namespace qtpyt {
    namespace {

        template <typename CallableType, typename AdditionalType>
        class QPySlotInternal : public QtPrivate::QSlotObjectBase {
        public:
            QPySlotInternal(QSharedPointer<CallableType> callable, QSharedPointer<QPyFutureNotifier> notifier, QMetaMethod method, void (*fn)(int which, QtPrivate::QSlotObjectBase* this_, QObject *receiver, void **args, bool *ret), const QPyRegisteredType& returnType, AdditionalType *additional = nullptr ) :
            QSlotObjectBase(fn), m_callable(std::move(callable)), m_notifier(std::move(notifier)), m_method(method), m_returnType(returnType), m_additional(additional) {
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

            auto module()  {
                if (!m_callable) {
                    qWarning() << "QPySlot: module is null for function" << m_functionName;
                }
                if (m_callable->functionName() != m_functionName) {
                    m_callable->setCallableFunction(m_functionName);
                }
                return m_callable;
            }

            void setCallableFunction(const QString& functionName) {
                m_functionName = functionName;
            }

            const QMetaMethod& method() const {
                return m_method;
            }

            [[nodiscard]] QSharedPointer<QPyFutureNotifier> notifier() const {
                return m_notifier;
            }

            [[nodiscard]] AdditionalType * additional() const {
                return m_additional;
            }

            [[nodiscard]] QString functionName() const {
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

            [[nodiscard]] QByteArray returnType() const {
                if (std::holds_alternative<QMetaType>(m_returnType)) {
                    return std::get<QMetaType>(m_returnType).name();
                } else if (std::holds_alternative<QString>(m_returnType)) {
                    return std::get<QString>(m_returnType).toUtf8();
                } else if (std::holds_alternative<QMetaType::Type>(m_returnType)) {
                    return QMetaType(std::get<QMetaType::Type>(m_returnType)).name();
                }
                return {};
            }

        private:
            QVector<int> m_parameterTypes;
            QSharedPointer<CallableType> m_callable;
            QMetaMethod m_method;
            QString m_functionName;
            QSharedPointer<QPyFutureNotifier> m_notifier;
            QPyRegisteredType m_returnType;
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
            slot->module()->call(argsTuple, {});
        } catch (const pybind11::error_already_set& e) {
            qWarning() << "Error in slot module:" << e.what();
        }
    };

     QMetaObject::Connection QPySlot::connectPythonFunction(QObject* sender, const char* signal, const QSharedPointer<QPyModuleBase> module,
     const QString&  slot,
     const QPyRegisteredType& returnType, Qt::ConnectionType type) {
        module->setCallableFunction(slot);
        const auto pyCallableInfo = module->inspectCallable();
        const auto signalMethod = findMatchingSignal(sender, signal, pyCallableInfo);
        if (!signalMethod) {
            qWarning() << "QPySlot::connectCallable: no matching signal found for" << signal;
            return {};
        }
        const auto mmethod = signalMethod.value();
        auto* slotObject = new QPySlotInternal<QPyModuleBase, void>(module, nullptr, mmethod, callProc, returnType);
        slotObject->setCallableFunction(slot);
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
            auto asyncModule = slot->module();

            if (asyncModule.get()) {
                QPyFuture f = QPyFuture(asyncModule, slot->notifier(), slot->functionName(), slot->returnType(), slot->parameterTypes(),  a);
                QPyThreadPool::instance().submit(std::move(f));
            } else {
                qWarning() << "QPySlot::connectCallableAsync: module is not a QPyModule";
            }
            this_->destroyIfLastRef();
        } catch (const pybind11::error_already_set& e) {
            qWarning() << "Error in async slot module:" << e.what();
        }
    };

    QMetaObject::Connection QPySlot::connectPythonFunctionAsync(QObject* sender, const char* signal, QSharedPointer<QPyModule> module, const QSharedPointer<QPyFutureNotifier>& notifier, const QString& slot, const QPyRegisteredType& returnType) {
        module->setCallableFunction(slot);
        const auto pyCallableInfo = module->inspectCallable();
        const auto signalMethod = findMatchingSignal(sender, signal, pyCallableInfo);
        if (!signalMethod) {
            qWarning() << "QPySlot::connectCallableAsync: no matching signal found for" << signal;
            return {};
        }
        const auto mmethod = signalMethod.value();
        //auto callableFunc = []
        auto* slotObject = new QPySlotInternal<QPyModule, void>(std::move(module), notifier, mmethod, callInThreadPool, returnType);
        slotObject->setCallableFunction(slot);
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

            if (auto asyncCallable = slot->module(); auto thread = slot->additional()) {
                //asyncCallable->bindArguments(argsTuple);
                //asyncCallable->callInThread(thread);
            } else {
                qWarning() << "QPySlot::connectCallableAsync: module is not a QPyModule";
            }
        } catch (const pybind11::error_already_set& e) {
            qWarning() << "Error in async slot module:" << e.what();
        }
    };

    QMetaObject::Connection QPySlot::connectPythonFunctionAsync(QObject* sender, const char* signal,
        QSharedPointer<QPyModule> module, QSharedPointer<QPyFutureNotifier> notifier, const char *slot, const QPyRegisteredType& returnType, QPyThread* thread) {
        module->setCallableFunction(slot);
        const auto pyCallableInfo = module->inspectCallable();
        const auto signalMethod = findMatchingSignal(sender, signal, pyCallableInfo);
        if (!signalMethod) {
            qWarning() << "QPySlot::connectCallableAsync: no matching signal found for" << signal;
            return {};
        }
        const auto mmethod = signalMethod.value();
        //auto callableFunc = []
        auto* slotObject = new QPySlotInternal<QPyModule, QPyThread>(std::move(module), std::move(notifier), mmethod, callInThread, returnType, thread);
        const int signalIndex = mmethod.methodIndex();
        return QObjectPrivate::connect(sender, signalIndex, slotObject, Qt::DirectConnection);
    }

    QMetaObject::Connection QPySlot::connectPythonFunction(QObject* sender, int signalIndex, QSharedPointer<QPyModuleBase> module,
                                  QSharedPointer<QPyFutureNotifier> notifier, const QString& slot, const QPyRegisteredType& returnType, Qt::ConnectionType type) {
        auto mmethod = sender->metaObject()->method(signalIndex);
        auto* slotObject = new QPySlotInternal<QPyModuleBase, void>(std::move(module), std::move(notifier), mmethod, callProc, returnType);
        slotObject->setCallableFunction(slot);
        return QObjectPrivate::connect(sender, signalIndex, slotObject, type);
    }

    QPySlot::QPySlot(QSharedPointer<QPyModule> module, QSharedPointer<QPyFutureNotifier> notifier,
        const QString &slotName, const QPyRegisteredType &returnType) : m_module(std::move(module)),
    m_notifier(std::move(notifier)), m_slotName(slotName), m_returnType(returnType)
    {}

    QMetaObject::Connection QPySlot::
    connectAsyncToSignal(QObject *sender, const char *signal, Qt::ConnectionType type) const {
        return connectPythonFunctionAsync(sender, signal, m_module, m_notifier, m_slotName, m_returnType);
    }

    QMetaObject::Connection QPySlot::
    connectToSignal(QObject *sender, const char *signal, Qt::ConnectionType type) const {
        return connectPythonFunction(sender, signal, m_module, m_slotName, m_returnType, type);
    }

    std::optional<QMetaMethod> QPySlot::findMatchingSignal(QObject* sender, const char* signal, const PyCallableInfo& pyCallableInfo) {
        const QMetaObject *mo = sender->metaObject();
        // Find method by name + arg count
        int methodIndex = -1;
        QByteArray qMethodName(signal);
        methodIndex = mo->indexOfSignal(QMetaObject::normalizedSignature(qPrintable(signal)));
        if (methodIndex < 0)    {
            for (int i = mo->methodOffset(); i < mo->methodCount(); ++i) {
                QMetaMethod m = mo->method(i);

                if (m.methodType() == QMetaMethod::Signal) {
                    auto extractName = [](const char* sig) -> QByteArray {
                        QByteArray ba(sig);
                        int parenIndex = ba.indexOf('(');
                        if (parenIndex >= 0) {
                            return ba.left(parenIndex);
                        }
                        return ba;
                    };
                    if (m.name() == extractName(signal)) {
                        if (m.parameterCount() == pyCallableInfo.arguments.size()) {
                            methodIndex = i;
                            break;
                        }
                    }
                }
            }
        }
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