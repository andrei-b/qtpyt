
#include "q_embed_meta_object.h"
#include <QMetaMethod>

namespace qtpyt {
    bool QEmbedMetaObject::invokeFromVariantListDynamic(QObject *obj, const char *methodName, const QVariantList &args,
        QVariant *outReturn, Qt::ConnectionType type) {
        if (!obj) {
            qWarning() << "invokeFromVariantListDynamic: null QObject";
            return false;
        }

        const QMetaObject *mo = obj->metaObject();

        // Find method by name + arg count
        int methodIndex = -1;
        QByteArray qMethodName(methodName);
        for (int i = 0; i < mo->methodCount(); ++i) {
            QMetaMethod m = mo->method(i);
            // match simple name or full signature (if caller passed "setWindowTitle(QString)")
            if ((m.name() == qMethodName || m.methodSignature() == qMethodName)
                && m.parameterCount() == args.size()) {
                methodIndex = i;
                break;
                }
        }

        if (methodIndex < 0) {
            qWarning() << "invokeFromVariantListDynamic: method not found:" << methodName
                    << "on" << mo->className();
            return false;
        }

        QMetaMethod mm = mo->method(methodIndex);

        if (args.size() > MAX_QVARIANT_ARGS) {
            qWarning() << "invokeFromVariantListDynamic: too many arguments:" << args.size();
            return false;
        }

        const int n = args.size();

        // --- Prepare argument storage ---
        QVector<void *> storage; // raw storage for converted args
        QVector<int> typeIds; // matching type ids for cleanup
        storage.reserve(n);
        typeIds.reserve(n);

        QVector<QGenericArgument> genArgs;
        genArgs.reserve(n);

        for (int i = 0; i < n; ++i) {
            int paramTypeId = mm.parameterType(i);
            if (paramTypeId == QMetaType::Void) {
                qWarning() << "invokeFromVariantListDynamic: unknown param type at index" << i;
                return false;
            }

            QVariant v = args[i];
            if (!v.isValid()) {
                // default-construct if invalid
                v = QVariant(static_cast<QMetaType>(paramTypeId), nullptr);
            }

            if (paramTypeId == QMetaType::QVariant) {
                auto *varPtr = new QVariant(v);
                storage.push_back(varPtr);
                typeIds.push_back(paramTypeId);
                genArgs.push_back(QGenericArgument("QVariant", varPtr));
                continue;
            }

            if (!v.convert(paramTypeId)) {
                qWarning() << "invokeFromVariantListDynamic: cannot convert arg" << i
                        << "from type" << v.typeName()
                        << "to param type" << QMetaType::typeName(paramTypeId);

                // cleanup already allocated storage
                for (int j = 0; j < storage.size(); ++j) {
                    if (typeIds[j] == QMetaType::QVariant) {
                        delete static_cast<QVariant *>(storage[j]);
                    } else {
                        QMetaType::destroy(typeIds[j], storage[j]);
                    }
                }
                return false;
            }

            void *ptr = QMetaType::create(paramTypeId, v.constData());
            storage.push_back(ptr);
            typeIds.push_back(paramTypeId);

            const char *typeName = QMetaType::typeName(paramTypeId);
            genArgs.push_back(QGenericArgument(typeName, ptr));
        }

        auto argAt = [&](int i) -> const QGenericArgument & {
            static QGenericArgument empty;
            return (i < genArgs.size()) ? genArgs[i] : empty;
        };

        // --- Prepare return value (if any) ---
        int returnTypeId = mm.returnType(); // works in Qt5 and Qt6 (deprecated there but fine)
        bool hasReturn = (returnTypeId != QMetaType::Void);

        void *retStorage = nullptr;
        QGenericReturnArgument retArg;

        if (hasReturn) {
            const char *retTypeName = QMetaType::typeName(returnTypeId);
            if (!retTypeName) {
                qWarning() << "invokeFromVariantListDynamic: unknown return type for method"
                        << methodName << "on" << mo->className();
                // cleanup args
                for (int j = 0; j < storage.size(); ++j) {
                    if (typeIds[j] == QMetaType::QVariant) {
                        delete static_cast<QVariant *>(storage[j]);
                    } else {
                        QMetaType::destroy(typeIds[j], storage[j]);
                    }
                }
                return false;
            }

            // default-construct return value
            retStorage = QMetaType::create(returnTypeId);
            retArg = QGenericReturnArgument(retTypeName, retStorage);
        }

        // --- Invoke ---
        bool ok = false;

        if (hasReturn) {
            // With return argument: retArg comes first
            switch (n) {
                case 0:
                    ok = mm.invoke(obj, type, retArg);
                    break;
                case 1:
                    ok = mm.invoke(obj, type, retArg,
                                   argAt(0));
                    break;
                case 2:
                    ok = mm.invoke(obj, type, retArg,
                                   argAt(0), argAt(1));
                    break;
                case 3:
                    ok = mm.invoke(obj, type, retArg,
                                   argAt(0), argAt(1), argAt(2));
                    break;
                case 4:
                    ok = mm.invoke(obj, type, retArg,
                                   argAt(0), argAt(1), argAt(2), argAt(3));
                    break;
                case 5:
                    ok = mm.invoke(obj, type, retArg,
                                   argAt(0), argAt(1), argAt(2), argAt(3), argAt(4));
                    break;
                case 6:
                    ok = mm.invoke(obj, type, retArg,
                                   argAt(0), argAt(1), argAt(2), argAt(3), argAt(4), argAt(5));
                    break;
                case 7:
                    ok = mm.invoke(obj, type, retArg,
                                   argAt(0), argAt(1), argAt(2), argAt(3), argAt(4), argAt(5), argAt(6));
                    break;
                case 8:
                    ok = mm.invoke(obj, type, retArg,
                                   argAt(0), argAt(1), argAt(2), argAt(3), argAt(4), argAt(5), argAt(6), argAt(7));
                    break;
                case 9:
                    ok = mm.invoke(obj, type, retArg,
                                   argAt(0), argAt(1), argAt(2), argAt(3), argAt(4), argAt(5), argAt(6), argAt(7),
                                   argAt(8));
                    break;
                case 10:
                    ok = mm.invoke(obj, type, retArg,
                                   argAt(0), argAt(1), argAt(2), argAt(3), argAt(4),
                                   argAt(5), argAt(6), argAt(7), argAt(8), argAt(9));
                    break;
                default:
                    qWarning() << "invokeFromVariantListDynamic: arg count not handled:" << n;
                    ok = false;
                    break;
            }
        } else {
            // No return value
            switch (n) {
                case 0:
                    ok = mm.invoke(obj, type);
                    break;
                case 1:
                    ok = mm.invoke(obj, type,
                                   argAt(0));
                    break;
                case 2:
                    ok = mm.invoke(obj, type,
                                   argAt(0), argAt(1));
                    break;
                case 3:
                    ok = mm.invoke(obj, type,
                                   argAt(0), argAt(1), argAt(2));
                    break;
                case 4:
                    ok = mm.invoke(obj, type,
                                   argAt(0), argAt(1), argAt(2), argAt(3));
                    break;
                case 5:
                    ok = mm.invoke(obj, type,
                                   argAt(0), argAt(1), argAt(2), argAt(3), argAt(4));
                    break;
                case 6:
                    ok = mm.invoke(obj, type,
                                   argAt(0), argAt(1), argAt(2), argAt(3), argAt(4), argAt(5));
                    break;
                case 7:
                    ok = mm.invoke(obj, type,
                                   argAt(0), argAt(1), argAt(2), argAt(3), argAt(4), argAt(5), argAt(6));
                    break;
                case 8:
                    ok = mm.invoke(obj, type,
                                   argAt(0), argAt(1), argAt(2), argAt(3), argAt(4), argAt(5), argAt(6), argAt(7));
                    break;
                case 9:
                    ok = mm.invoke(obj, type,
                                   argAt(0), argAt(1), argAt(2), argAt(3), argAt(4), argAt(5), argAt(6), argAt(7),
                                   argAt(8));
                    break;
                case 10:
                    ok = mm.invoke(obj, type,
                                   argAt(0), argAt(1), argAt(2), argAt(3), argAt(4),
                                   argAt(5), argAt(6), argAt(7), argAt(8), argAt(9));
                    break;
                default:
                    qWarning() << "invokeFromVariantListDynamic: arg count not handled:" << n;
                    ok = false;
                    break;
            }
        }

        // --- Extract return value (if requested) ---
        if (hasReturn) {
            if (ok && outReturn) {
                // construct QVariant from the returned value
                *outReturn = QVariant(static_cast<QMetaType>(returnTypeId), retStorage);
            }
            QMetaType::destroy(returnTypeId, retStorage);
        }

        // --- cleanup arguments ---
        for (int i = 0; i < storage.size(); ++i) {
            if (typeIds[i] == QMetaType::QVariant) {
                delete static_cast<QVariant *>(storage[i]);
            } else {
                QMetaType::destroy(typeIds[i], storage[i]);
            }
        }

        if (!ok) {
            qWarning() << "invokeFromVariantListDynamic: invoke failed for"
                    << methodName << "on" << mo->className();
        }

        return ok;
    }
} // namespace qtpyt