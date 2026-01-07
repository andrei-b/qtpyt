#pragma once

#include <qtpyt/qpymodule.h>
#include <qtpyt/qpymodulebase.h>
#include <qtpyt/q_py_thread.h>
#include <utility>
#include <private/qmetaobject_p.h>
#include <private/qobject_p.h>

namespace qtpyt {
    void impl(int which, QtPrivate::QSlotObjectBase* this_, QObject* r, void** a, bool* ret);

    class QPySlot {
    public:
        static QMetaObject::Connection connectPythonFunction(QObject *sender, const char *signal,
                                                             QPyModuleBase module,
                                                             const QString &slot, const QPyRegisteredType &returnType = QMetaType::Void, Qt::ConnectionType type =
                                                                     Qt::AutoConnection);
        static QMetaObject::Connection connectPythonFunction(QObject *sender, const char *signal,
                                                             QPyModule module,
                                                             const QString &slot, const QPyRegisteredType &returnType = QMetaType::Void, Qt::ConnectionType type =
                                                                     Qt::AutoConnection);

        static QMetaObject::Connection connectPythonFunctionAsync(QObject *sender, const char *signal,
                                                                  QPyModule module,
                                                                  const QSharedPointer<IQPyFutureNotifier> &notifier,
                                                                  const QString &slot, const QPyRegisteredType &returnType = QMetaType::Void);
        static QMetaObject::Connection connectPythonFunctionAsync(QObject *sender, const char *signal,
                                                                  QPyModule module, QSharedPointer<IQPyFutureNotifier> notifier,
                                                                  const char *slot,
                                                                  const QPyRegisteredType &returnType, QPyThread *thread);
        template <typename SignalFunc>
        static QMetaObject::Connection connectPythonFunction(const typename QtPrivate::FunctionPointer<SignalFunc>::Object* sender,
                                                SignalFunc signal,
                                                QSharedPointer<QPyModuleBase> module,
                                                QSharedPointer<IQPyFutureNotifier> notifier,
                                                const QString& slot,
                                                const QPyRegisteredType& returnType = QMetaType::Void,
                                                Qt::ConnectionType type = Qt::AutoConnection) {
            int signalIndex =  getMethodIndex<SignalFunc>(sender, signal).value_or(-1);
            if (signalIndex < 0) {
                qWarning("connectCallable: cannot match the signal");
                return {};
            }
            return connectPythonFunction(sender, signalIndex, module, notifier, slot, returnType, type);
        }

        template <typename SignalFunc>
        static QMetaObject::Connection connectPythonFunctionAsync(QObject * sender, const SignalFunc signal, std::shared_ptr<QPyModule> module, QSharedPointer<IQPyFutureNotifier> notifier, const QPyRegisteredType& returnType = QMetaType::Void) {
            int signalIndex =  getMethodIndex<SignalFunc>(sender, signal).value_or(-1);
            if (signalIndex < 0) {
                qWarning("connectCallableAsync: cannot match the signal");
                return {};
            }
            const char* signalSignature = sender->metaObject()->method(signalIndex).methodSignature().constData();
            return connectPythonFunctionAsync(sender, signalSignature, std::move(module), std::move(notifier), returnType);
        }

        template <typename SignalFunc>
        static QMetaObject::Connection connectPythonFunctionAsync(const typename QtPrivate::FunctionPointer<SignalFunc>::Object* sender,
                                                SignalFunc signal, std::shared_ptr<QPyModule> module,
                                                const QPyRegisteredType& returnType, QPyThread* thread) {
            int signalIndex =  getMethodIndex<SignalFunc>(sender, signal).value_or(-1);
            if (signalIndex < 0) {
                qWarning("connectCallableAsync: cannot match the signal");
                return {};
            }
            const char* signalSignature = sender->metaObject()->method(signalIndex).methodSignature().constData();
            return connectPythonFunctionAsync(sender, signalSignature, module, returnType, thread);
        }


        template <typename SignalFunc>
        static QMetaObject::Connection connectPythonFunction(const typename QtPrivate::FunctionPointer<SignalFunc>::Object* sender,
                                                SignalFunc signal,
                                                std::shared_ptr<QPyModule> module,
                                                QSharedPointer<IQPyFutureNotifier> notifier,
                                                const QString& slot,
                                                const QPyRegisteredType& returnType) {
            int signalIndex =  getMethodIndex<SignalFunc>(sender, signal).value_or(-1);
            if (signalIndex < 0) {
                qWarning("connectCallable: cannot match the signal");
                return {};
            }
            return connectPythonFunction(sender, signalIndex, module, std::move(notifier), slot, returnType);
        }

        static QMetaObject::Connection connectPythonFunction(QObject *sender, int signalIndex, QPyModuleBase module,
                                                             QSharedPointer<IQPyFutureNotifier> notifier, const QString &slot, const QPyRegisteredType &returnType, Qt::
                                                             ConnectionType type =
                                                                     Qt::AutoConnection);

        static void connect(QObject* sender, const char* signal, Qt::ConnectionType type) {
            int i = 0;
            auto impl2 = [](int which, QtPrivate::QSlotObjectBase* this_, QObject* r, void** a, bool* ret) {
                qDebug() << "QPySlot called from lambda";
            };
            auto* slotObject = new QtPrivate::QSlotObjectBase(impl2);
            const int signalIndex = QObjectPrivate::get(sender)->signalIndex(signal);
            QObjectPrivate::connect(sender, signalIndex, slotObject, type);
        }
        QPySlot(QPyModule module, QSharedPointer<IQPyFutureNotifier> notifier, const QString& slotName,
            const QPyRegisteredType& returnType);

        QMetaObject::Connection connectAsyncToSignal(QObject *sender, const char *signal, Qt::ConnectionType type = Qt::AutoConnection) const;
        QMetaObject::Connection connectToSignal(QObject* sender, const char* signal, Qt::ConnectionType type = Qt::AutoConnection) const;
        template <typename SignalFunc>
        QMetaObject::Connection connectAsyncToSignal(typename QtPrivate::FunctionPointer<SignalFunc>::Object* sender,
                                                SignalFunc signal,
                                                Qt::ConnectionType type = Qt::AutoConnection) const {
            int signalIndex =  getMethodIndex<SignalFunc>(sender, signal).value_or(-1);
            if (signalIndex < 0) {
                qWarning("connectAsyncToSignal: cannot match the signal");
                return {};
            }
            const char* signalSignature = sender->metaObject()->method(signalIndex).methodSignature().constData();
            return connectAsyncToSignal(sender, signalSignature, type);
        }
        template <typename SignalFunc>
        QMetaObject::Connection connectToSignal(const typename QtPrivate::FunctionPointer<SignalFunc>::Object* sender,
                                                SignalFunc signal,
                                                Qt::ConnectionType type = Qt::AutoConnection) const {
            int signalIndex =  getMethodIndex<SignalFunc>(sender, signal).value_or(-1);
            if (signalIndex < 0) {
                qWarning("connectToSignal: cannot match the signal");
                return {};
            }
            const char* signalSignature = sender->metaObject()->method(signalIndex).methodSignature().constData();
            return connectToSignal(const_cast<QObject*>(static_cast<const QObject*>(sender)), signalSignature, type);
        }
    private:
        template <typename SignalFunc>
        static std::optional<int> getSignalIndex(const QObject* sender, SignalFunc signal) {
            if (!signal) {
                qWarning("QObject::connect: invalid nullptr parameter");
                return std::nullopt;
            }

            int localIndex = -1;
            SignalFunc signalVar = signal;
            void* args[] = { &localIndex, reinterpret_cast<void*>(&signalVar) };

            const QMetaObject* originalMeta = sender->metaObject();
            const QMetaObject* foundMeta = nullptr;

            for (const QMetaObject* m = originalMeta; m && localIndex < 0; m = m->superClass()) {
                m->static_metacall(QMetaObject::IndexOfMethod, 0, args);
                if (localIndex >= 0 && localIndex < QMetaObjectPrivate::get(m)->signalCount) {
                    foundMeta = m;
                    break;
                }
            }

            if (!foundMeta) {
                qWarning("QObject::connect: signal not found in %s", originalMeta->className());
                return std::nullopt;
            }

            // convert the local index in the declaring meta-object to the absolute method index
            int absoluteMethodIndex = foundMeta->methodOffset() + localIndex;
            if (absoluteMethodIndex < 0 || absoluteMethodIndex >= originalMeta->methodCount()) {
                qWarning("QObject::connect: computed method index out of range for %s", foundMeta->className());
                return std::nullopt;
            }

            // use the original meta-object to get the QMetaMethod for the absolute index
            QMetaMethod declaringMethod = originalMeta->method(absoluteMethodIndex);
            QByteArray declaringSig = declaringMethod.methodSignature(); // keep QByteArray alive

            // ask the actual sender instance for the signal index (index valid for sender)
            int senderSignalIndex = QObjectPrivate::get(const_cast<QObject*>(sender))->signalIndex(declaringSig.constData());
            if (senderSignalIndex < 0) {
                qWarning("QObject::connect: failed to map signature '%s' to sender %s", declaringSig.constData(), originalMeta->className());
                return std::nullopt;
            }

            return senderSignalIndex;
        }

        template <typename SignalFunc>
    static std::optional<int> getMethodIndex(const QObject* sender, SignalFunc signal) {
            if (!signal) {
                qWarning("QObject::connect: invalid nullptr parameter");
                return std::nullopt;
            }

            int localIndex = -1;
            SignalFunc signalVar = signal;
            void* args[] = { &localIndex, reinterpret_cast<void*>(&signalVar) };

            const QMetaObject* originalMeta = sender->metaObject();
            const QMetaObject* foundMeta = nullptr;

            for (const QMetaObject* m = originalMeta; m && localIndex < 0; m = m->superClass()) {
                m->static_metacall(QMetaObject::IndexOfMethod, 0, args);
                if (localIndex >= 0 && localIndex < QMetaObjectPrivate::get(m)->signalCount) {
                    foundMeta = m;
                    break;
                }
            }

            if (!foundMeta) {
                qWarning("QObject::connect: signal not found in %s", originalMeta->className());
                return std::nullopt;
            }

            int absoluteMethodIndex = foundMeta->methodOffset() + localIndex;
            if (absoluteMethodIndex < 0 || absoluteMethodIndex >= originalMeta->methodCount()) {
                qWarning("QObject::connect: computed method index out of range for %s", foundMeta->className());
                return std::nullopt;
            }

            return absoluteMethodIndex;
        }
        static std::optional<QMetaMethod> findMatchingSignal(QObject* sender, const char* signal,
        const PyCallableInfo& pyCallableInfo);

        QPyModule m_module;
        QSharedPointer<IQPyFutureNotifier> m_notifier;
        QString m_slotName;
        QPyRegisteredType m_returnType;
    };
} // namespace qtpyt