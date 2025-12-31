#pragma once

#include "q_embed_meta_object_py.h"
#include <qtpyt/qpymodule.h>
#include <qtpyt/qpymodulebase.h>
#include <qtpyt/q_py_thread.h>
#include <QThreadPool>
#include <utility>
#include <private/qmetaobject_p.h>
#include <private/qobject_p.h>
#include <pybind11/common.h>

namespace qtpyt {
    void impl(int which, QtPrivate::QSlotObjectBase* this_, QObject* r, void** a, bool* ret);

    class QPySlot {
    public:
        static QMetaObject::Connection connectCallable(QObject *sender, const char *signal,
                                                       QSharedPointer<QPyModuleBase> callable,
                                                       Qt::ConnectionType type = Qt::AutoConnection);
        static QMetaObject::Connection connectCallableAsync(QObject *sender, const char *signal,
                                                            QSharedPointer<QPyModule> callable, QSharedPointer<QPyFutureNotifier> notifier);
        static QMetaObject::Connection connectCallableAsync(QObject *sender, const char *signal,
                                                            QSharedPointer<QPyModule> callable, QSharedPointer<QPyFutureNotifier> notifier, QPyThread *thread);
        template <typename SignalFunc>
        static QMetaObject::Connection connectCallable(const typename QtPrivate::FunctionPointer<SignalFunc>::Object* sender,
                                                SignalFunc signal,
                                                QSharedPointer<QPyModuleBase> callable,
                                                QSharedPointer<QPyFutureNotifier> notifier,
                                                Qt::ConnectionType type) {
            int signalIndex =  getMethodIndex<SignalFunc>(sender, signal).value_or(-1);
            if (signalIndex < 0) {
                qWarning("connectCallable: cannot match the signal");
                return {};
            }
            return connectCallable(sender, signalIndex, callable, notifier, type);
        }

        template <typename SignalFunc>
        static QMetaObject::Connection connectCallableAsync(QObject * sender, const SignalFunc signal, std::shared_ptr<QPyModule> module, QSharedPointer<QPyFutureNotifier> notifier) {
            int signalIndex =  getMethodIndex<SignalFunc>(sender, signal).value_or(-1);
            if (signalIndex < 0) {
                qWarning("connectCallableAsync: cannot match the signal");
                return {};
            }
            const char* signalSignature = sender->metaObject()->method(signalIndex).methodSignature().constData();
            return connectCallableAsync(sender, signalSignature, std::move(module), std::move(notifier));
        }

        template <typename SignalFunc>
        static QMetaObject::Connection connectCallableAsync(const typename QtPrivate::FunctionPointer<SignalFunc>::Object* sender,
                                                SignalFunc signal, std::shared_ptr<QPyModule> module, QSharedPointer<QPyFutureNotifier> notifier, QPyThread* thread) {
            int signalIndex =  getMethodIndex<SignalFunc>(sender, signal).value_or(-1);
            if (signalIndex < 0) {
                qWarning("connectCallableAsync: cannot match the signal");
                return {};
            }
            const char* signalSignature = sender->metaObject()->method(signalIndex).methodSignature().constData();
            return connectCallableAsync(sender, signalSignature, module, std::move(notifier), thread);
        }

        static QMetaObject::Connection connectCallable(QObject *sender, int signalIndex, QSharedPointer<QPyModuleBase> module,
                                                       QSharedPointer<QPyFutureNotifier> notifier, Qt::ConnectionType type = Qt::AutoConnection);

        static void connect(QObject* sender, const char* signal, Qt::ConnectionType type) {
            int i = 0;
            auto impl2 = [](int which, QtPrivate::QSlotObjectBase* this_, QObject* r, void** a, bool* ret) {
                qDebug() << "QPySlot called from lambda";
            };
            auto* slotObject = new QtPrivate::QSlotObjectBase(impl2);
            const int signalIndex = QObjectPrivate::get(sender)->signalIndex(signal);
            QObjectPrivate::connect(sender, signalIndex, slotObject, type);
        }

    private:
        // file: `slot.h`
        // Fix: use originalMeta->method(...) and check against originalMeta->methodCount()
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
    };
} // namespace qtpyt