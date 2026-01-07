#pragma once

#include <QMetaType>
#include <QVariant>
#include <QVector>
#include <QGenericArgument>
#include <QDebug>
#include <QString>

#include <any>
#include <array>
#include <vector>
#include <utility>
#include <type_traits>

namespace qtpyt {
    const int MAX_QVARIANT_ARGS = 10; // Qt supports up to 10 arguments in QMetaObject::invokeMethod

    class QEmbedMetaObject {
    public:
        template<typename... Args>
        static bool invokeMethodVoid(QObject *object,
                                     const char *method,
                                     Qt::ConnectionType type,
                                     Args &&... args) {
            static_assert(sizeof...(Args) <= MAX_QVARIANT_ARGS,
                          "Qt invokeMethod supports max 10 arguments");

            std::vector<std::any> storage;
            storage.reserve(sizeof...(Args)); // keep addresses stable across QGenericArgument construction

            auto makeArg = [&](auto &&v) -> QGenericArgument {
                using T = std::decay_t<decltype(v)>;

                if constexpr (std::is_same_v<T, QVariant>) {
                    storage.emplace_back(std::forward<decltype(v)>(v));
                    return {"QVariant", &std::any_cast<QVariant &>(storage.back())};
                } else if constexpr (std::is_same_v<T, const char *> || std::is_same_v<T, char *>) {
                    storage.emplace_back(QString::fromUtf8(v));
                    return {"QString", &std::any_cast<QString &>(storage.back())};
                } else if constexpr (std::is_convertible_v<T, QString> && !std::is_same_v<T, QString>) {
                    storage.emplace_back(QString::fromStdString(std::string(v)));
                    return {"QString", &std::any_cast<QString &>(storage.back())};
                } else {
                    storage.emplace_back(std::forward<decltype(v)>(v));
                    auto meta = QMetaType::fromType<T>();
                    const char *typeName = meta.name();
                    return {
                        typeName,
                        static_cast<const void *>(std::addressof(std::any_cast<T &>(storage.back())))
                    };
                }
            };

            auto genericArgs = std::array<QGenericArgument, sizeof...(Args)>{makeArg(std::forward<Args>(args))...};

            auto call = [&](auto &... unpacked) {
                return QMetaObject::invokeMethod(
                    object,
                    method,
                    type,
                    unpacked...
                );
            };

            return std::apply(
                [&](auto &... elem) { return call(elem...); },
                genericArgs
            );
        }

        static bool invokeFromVariantListDynamic(QObject *obj,
                                                 const char *methodName,
                                                 const QVariantList &args,
                                                 QVariant *outReturn = nullptr,
                                                 Qt::ConnectionType type = Qt::AutoConnection);

        static bool setProperty(QObject* obj, const QString& name, const QVariant& value)
        {
            return obj->setProperty(name.toUtf8().constData(), value);
        }


        static QVariant getProperty(QObject* obj, const QString& name)
        {
            return obj->property(name.toUtf8().constData());
        }

    private:
        // no toGenericArg member required here; the template method above uses a lambda-local generator
        QEmbedMetaObject() = delete;
    };

}// namespace qtpyt