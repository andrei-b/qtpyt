#define  PYBIND11_NO_KEYWORDS
#include <pybind11/pybind11.h>

#include "slot.h"

#include <qtpyt/conversions.h>
#include <qtpyt/q_py_thread_pool.h>
#include <QMetaMethod>

namespace qtpyt {
    namespace {

        template <typename CallableType, typename AdditionalType>
        class QPySlot : public QtPrivate::QSlotObjectBase {
        public:
            QPySlot(std::shared_ptr<CallableType> callable, QMetaMethod method, void (*fn)(int which, QtPrivate::QSlotObjectBase* this_, QObject *receiver, void **args, bool *ret), AdditionalType *additional = nullptr ) :
            QSlotObjectBase(fn), m_callable(std::move(callable)), m_method(method), m_additional(additional) {
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
            std::shared_ptr<CallableType> m_callable;
            QMetaMethod m_method;
            QString m_functionName;
            AdditionalType * m_additional;
        };

    } // namespace

    void callProc(int which, QtPrivate::QSlotObjectBase* this_, QObject* r, void** a, bool* ret)  {
        if (which == 0) {
            return;
        }
        try {
            auto* slot = static_cast<QPySlot<QPyModuleBase, void>*>(this_);
            const auto argsTuple = slot->makeArgsTuple(a);
            slot->callable()->call(argsTuple, {});
        } catch (const pybind11::error_already_set& e) {
            qWarning() << "Error in slot callable:" << e.what();
        }
    };

    QMetaObject::Connection Slot::connectCallable(QObject* sender, const char* signal, const std::shared_ptr<QPyModuleBase> callable, Qt::ConnectionType type) {
        const auto pyCallableInfo = callable->inspectCallable();
        const auto signalMethod = findMatchingSignal(sender, signal, pyCallableInfo);
        if (!signalMethod) {
            qWarning() << "Slot::connectCallable: no matching signal found for" << signal;
            return {};
        }
        const auto mmethod = signalMethod.value();
        auto* slotObject = new QPySlot<QPyModuleBase, void>(std::move(callable), mmethod, callProc);
        const int signalIndex = mmethod.methodIndex();
        return QObjectPrivate::connect(sender, signalIndex, slotObject, type);
    }

    void callInThreadPool(int which, QtPrivate::QSlotObjectBase* this_, QObject* r, void** a, bool* ret)  {
        if (which == 0) {
            this_->destroyIfLastRef();
            return;
        }
        try {
            auto* slot = static_cast<QPySlot<QPyModule, void>*>(this_);
            auto asyncCallable = slot->callable();

            if (asyncCallable.get()) {
                QPyFuture f = QPyFuture(asyncCallable, slot->functionName(), QByteArray("void"), slot->parameterTypes(),  a);
                QPyThreadPool::instance().submit(std::move(f));
            } else {
                qWarning() << "Slot::connectCallableAsync: callable is not a QPyModule";
            }
            this_->destroyIfLastRef();
        } catch (const pybind11::error_already_set& e) {
            qWarning() << "Error in async slot callable:" << e.what();
        }
    };

    QMetaObject::Connection Slot::connectCallableAsync(QObject* sender, const char* signal, std::shared_ptr<QPyModule> callable) {
        const auto pyCallableInfo = callable->inspectCallable();
        const auto signalMethod = findMatchingSignal(sender, signal, pyCallableInfo);
        if (!signalMethod) {
            qWarning() << "Slot::connectCallableAsync: no matching signal found for" << signal;
            return {};
        }
        const auto mmethod = signalMethod.value();
        //auto callableFunc = []
        auto* slotObject = new QPySlot<QPyModule, void>(std::move(callable), mmethod, callInThreadPool);
        const int signalIndex = mmethod.methodIndex();
        return QObjectPrivate::connect(sender, signalIndex, slotObject, Qt::DirectConnection);
    }

    void callInThread(int which, QtPrivate::QSlotObjectBase* this_, QObject* r, void** a, bool* ret)  {
        if (which == 0) {
            return;
        }
        try {
            auto* slot = static_cast<QPySlot<QPyModule, QPyThread>*>(this_);
            const auto argsTuple = slot->makeArgsTuple(a);

            if (auto asyncCallable = slot->callable(); auto thread = slot->additional()) {
                //asyncCallable->bindArguments(argsTuple);
                //asyncCallable->callInThread(thread);
            } else {
                qWarning() << "Slot::connectCallableAsync: callable is not a QPyModule";
            }
        } catch (const pybind11::error_already_set& e) {
            qWarning() << "Error in async slot callable:" << e.what();
        }
    };

    QMetaObject::Connection Slot::connectCallableAsync(QObject* sender, const char* signal, std::shared_ptr<QPyModule> callable,
                                   QPyThread* thread) {
        const auto pyCallableInfo = callable->inspectCallable();
        const auto signalMethod = findMatchingSignal(sender, signal, pyCallableInfo);
        if (!signalMethod) {
            qWarning() << "Slot::connectCallableAsync: no matching signal found for" << signal;
            return {};
        }
        const auto mmethod = signalMethod.value();
        //auto callableFunc = []
        auto* slotObject = new QPySlot<QPyModule, QPyThread>(std::move(callable), mmethod, callInThread, thread);
        const int signalIndex = mmethod.methodIndex();
        return QObjectPrivate::connect(sender, signalIndex, slotObject, Qt::DirectConnection);
    }

    QMetaObject::Connection Slot::connectCallable(QObject* sender, int signalIndex, std::shared_ptr<QPyModuleBase> callable,
                                                  Qt::ConnectionType type) {
        auto mmethod = sender->metaObject()->method(signalIndex);
        auto* slotObject = new QPySlot<QPyModuleBase, void>(std::move(callable), mmethod, callProc);
        return QObjectPrivate::connect(sender, signalIndex, slotObject, type);
    }

    std::optional<QMetaMethod> Slot::findMatchingSignal(QObject* sender, const char* signal, const PyCallableInfo& pyCallableInfo) {
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
            qWarning() << "Slot::findMatchingSignal: no matching signal " << signal << "num args: " << pyCallableInfo.arguments.size()
                    << "on " << mo->className();
            return {};
        }
        return mo->method(methodIndex);
    }
} // namespace qtpyt