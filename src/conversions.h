#pragma once
#include <pybind11/pybind11.h>
#include <QVariant>
namespace py = pybind11;

namespace qtpyt {
    using PyObjectFromQVariantFunc = std::function<py::object(const QVariant &)>;

    std::optional<QVariant> pyObjectToQVariant(const py::handle &obj, const QByteArray &expectedType = {});

    QVariantList pySequenceToVariantList(const py::iterable &seq);

    QVariantMap pyDictToVariantMap(const py::dict &d);

    using ValueFromStringFunc = std::function<QVariant(const QString &)>;
    using ValueFromSequenceFunc = std::function<QVariant(py::sequence &)>;
    using ValueFromDictFunc = std::function<QVariant(py::dict &)>;
    using PyObjectFromVoidPtrFunc = std::function<py::object(const void *)>;
    using QVariantFromPyObjectFunc = std::function<QVariant(const py::object &)>;

    void addFromSequenceFunc(const QString &typeName, ValueFromSequenceFunc &&func);

    void addMetatypeVoidPtrToPyObjectConverterFunc(QMetaType::Type type, PyObjectFromVoidPtrFunc &&func);

    void addSpecializedMetatypeConverter(int typeId, QVariantFromPyObjectFunc &&func);

    template <typename T>
       QByteArray qtTypeNameOf() {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        const QMetaType mt = QMetaType::fromType<T>();
        return mt.isValid() ? QByteArray(mt.name()) : QByteArray{};
#else
        const int id = qMetaTypeId<T>();
        const char* n = QMetaType::typeName(id);
        return n ? QByteArray(n) : QByteArray{};
#endif
    }

    template<typename Container>
    void addSequenceFromSequenceConverter(const QString &typeName) {
        using T = typename Container::value_type;
        ValueFromSequenceFunc f = [](py::sequence &seq) -> QVariant {
            Container c;
            for (auto item: seq) {
                auto qItem = pyObjectToQVariant(item, qtTypeNameOf<T>());
                *std::inserter(c, c.end()) = qItem.value().template value<T>();
            }
            return QVariant::fromValue(c);
        };
        addFromSequenceFunc(typeName, std::move(f));
    }


    template<typename T>
    struct is_pair_like : std::false_type {
    };

    template<typename A, typename B>
    struct is_pair_like<std::pair<A, B> > : std::true_type {
    };



    py::object qvariantToPyObject(const QVariant &var);

    py::object qmetatypeToPyObject(int typeId, const void *data);

    void addFromQVariantFunc(int typeId, PyObjectFromQVariantFunc &&func);

    template<typename Container>
    void addSequenceToPyListConverter(int typeId) {
        using T = typename Container::value_type;
        PyObjectFromQVariantFunc f = [](const QVariant &v) {
            const auto container = v.template value<Container>();
            py::list t;
            for (const auto &elem: container) t.append(py::cast(elem));
            return t;
        };
        addFromQVariantFunc(typeId, std::move(f));
    }

    template<typename Container>
    using container_value_t = typename Container::value_type;

    template<typename Container>
    constexpr bool container_value_is_qvariant_v = std::is_same_v<container_value_t<Container>, QVariant>;

    template<typename Container>
    constexpr bool associative_mapped_is_qvariant_v =
            // only valid for pair-like value_types
            (is_pair_like<container_value_t<Container> >::value &&
             std::is_same_v<typename container_value_t<Container>::second_type, QVariant>);

    template<typename Container>
    void addAssociativeToPyDictConverter(int typeId) {
        using value_type = typename Container::value_type;

        static_assert(is_pair_like<value_type>::value,
                      "addAssociativeToPyDictConverter requires a pair-like Container::value_type");

        PyObjectFromQVariantFunc f = [](const QVariant &v) {
            const auto container = v.template value<Container>();
            py::dict d;
            for (const auto &p: container) {
                const QVariant &qtKey = p.first;
                const py::object key = qvariantToPyObject(qtKey);
                const QVariant &qtVal = p.second;
                py::object val = qvariantToPyObject(qtVal);
                d[key] = val;
            }
            return d;
        };
        addFromQVariantFunc(typeId, std::move(f));
    }

    template<typename Container>
    void addSequenceToPyTupleConverter(int typeId) {
        PyObjectFromQVariantFunc f = [](const QVariant &v) {
            const auto container = v.template value<Container>();
            auto first = std::begin(container);
            auto last = std::end(container);
            py::tuple t(static_cast<py::ssize_t>(std::distance(first, last)));
            py::ssize_t i = 0;
            for (auto it = first; it != last; ++it, ++i) {
                QVariant var(*it);
                auto val = qvariantToPyObject(var);
                t[i] = val;
            }
            return t;
        };
        addFromQVariantFunc(typeId, std::move(f));

    }

    template<typename Container>
    int registerContainerType(const QString &name) {
        auto newId = qRegisterMetaType<Container>(name.toStdString().c_str());
        addSequenceFromSequenceConverter<Container>(name);
        if constexpr (is_pair_like<typename Container::value_type>::value) {
            addAssociativeToPyDictConverter<Container>(newId);
        } else {
            addSequenceToPyTupleConverter<Container>(newId);
        }
        return newId;
    }

    void addFromDictFunc(const QString& name, ValueFromDictFunc&&func);

    template<typename Key, typename Value>
    int registerQMapType(const QString &name) {
        using MapType = QMap<Key, Value>;
        auto newId = qRegisterMetaType<MapType>(name.toStdString().c_str());
        PyObjectFromQVariantFunc f = [](const QVariant &v) {
            const auto map = v.template value<MapType>();
            py::dict d;
            for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
                const QVariant &qtKey = QVariant::fromValue(it.key());
                const py::object key = qvariantToPyObject(qtKey);
                const QVariant &qtVal = QVariant::fromValue(it.value());
                py::object val = qvariantToPyObject(qtVal);
                d[key] = val;
            }
            return d;
        };
        addFromQVariantFunc(newId, std::move(f));
        ValueFromSequenceFunc g = [](py::sequence &s) -> QVariant {
            MapType result;

            const py::handle h = s;

            // Case 1: dict -> QMap
            if (py::isinstance<py::dict>(h)) {
                for (const auto d = py::reinterpret_borrow<py::dict>(h); auto item: d) {
                    const py::handle k = item.first;
                    const py::handle v = item.second;

                    result.insert(k.cast<Key>(), v.cast<Value>());
                }
                return QVariant::fromValue(result);
            }

            QByteArray keyType = qtTypeNameOf<Key>();
            QByteArray valueType = qtTypeNameOf<Value>();
            // Case 2: iterable of pairs -> QMap
            for (const py::handle item: s) {
                auto pair = py::reinterpret_borrow<py::sequence>(item);
                if (py::len(pair) != 2) {
                    throw py::type_error("QMap converter expects dict or iterable of 2-item pairs");
                }
                const auto qKey = pyObjectToQVariant(pair[0], keyType);
                const auto qValue = pyObjectToQVariant(pair[1], valueType);
                if (!qKey.has_value() || !qValue.has_value()) {
                    throw std::runtime_error("Failed to convert py::dic to QMap");
                }
                result.insert(qKey.value().value<Key>(), qValue.value().value<Value>());
            }

            return QVariant::fromValue(result);
        };
        addFromSequenceFunc(name.toStdString().c_str(), std::move(g));

        ValueFromDictFunc k = [](py::dict &d) -> QVariant {
            MapType result;
            QByteArray keyType = qtTypeNameOf<Key>();
            QByteArray valueType = qtTypeNameOf<Value>();
            for (const auto item: d) {
                const auto qKey = pyObjectToQVariant(item.first, keyType);
                const auto qValue = pyObjectToQVariant(item.second, valueType);
                if (!qKey.has_value() || !qValue.has_value()) {
                    throw std::runtime_error("Failed to convert py::dic to QMap");
                }
                result.insert(qKey.value().value<Key>(), qValue.value().value<Value>());
            }
            return QVariant::fromValue(result);
        };

        addFromDictFunc(name.toStdString().c_str(), std::move(k));

        PyObjectFromVoidPtrFunc h = [](const void *data) -> py::object {
            if (!data) return py::none();

            const auto *map = static_cast<const MapType *>(data);

            py::dict d;
            for (auto it = map->constBegin(); it != map->constEnd(); ++it) {
                d[qvariantToPyObject(it.key())] = qvariantToPyObject(it.value());
            }
            return d;
        };
        addMetatypeVoidPtrToPyObjectConverterFunc(static_cast<QMetaType::Type>(newId), std::move(h));
        return newId;
    }

    int registerQHashType(const QString &name);

    int registerQSetType(const QString &name);

    std::optional<QVariant> pyObjectToQVariant(const py::handle &obj, int typeId);

    QVector<int> parameterTypeIds(const QList<QByteArray> &paramTypes);

    void addFromPyObjectToQVariantFunc(const QString &name, QVariantFromPyObjectFunc &&func);

    template <typename First, typename Second>
    int registerQPairType(const QString &name) {
        using PairType = QPair<First, Second>;
        auto newId = qRegisterMetaType<PairType>(name.toStdString().c_str());
        ValueFromSequenceFunc f = [](py::sequence &s) -> QVariant {
            if (py::len(s) != 2) {
                throw py::type_error("QPair converter expects a sequence of length 2");
            }
            const auto firstOpt = pyObjectToQVariant(s[0], qtTypeNameOf<First>());
            const auto secondOpt = pyObjectToQVariant(s[1], qtTypeNameOf<Second>());
            if (!firstOpt.has_value() || !secondOpt.has_value()) {
                throw std::runtime_error("Failed to convert py::sequence to QPair");
            }
            PairType p;
            p.first = firstOpt.value().template value<First>();
            p.second = secondOpt.value().template value<Second>();
            return QVariant::fromValue(p);
        };
        addFromSequenceFunc(name.toStdString().c_str(), std::move(f));
        ValueFromDictFunc g = [](py::dict &d) -> QVariant {
            PairType p;
            QByteArray firstType = qtTypeNameOf<First>();
            QByteArray secondType = qtTypeNameOf<Second>();
            if (d.contains("first")) {
                const auto firstOpt = pyObjectToQVariant(d["first"], firstType);
                if (!firstOpt.has_value()) {
                    throw std::runtime_error("Failed to convert 'first' key to QPair");
                }
                p.first = firstOpt.value().template value<First>();
            }
            if (d.contains("second")) {
                const auto secondOpt = pyObjectToQVariant(d["second"], secondType);
                if (!secondOpt.has_value()) {
                    throw std::runtime_error("Failed to convert 'second' key to QPair");
                }
                p.second = secondOpt.value().template value<Second>();
            }
            return QVariant::fromValue(p);
        };
        addFromDictFunc(name.toStdString().c_str(), std::move(g));
        PyObjectFromQVariantFunc h = [](const QVariant &v) {
            const auto &pair = v.template value<PairType>();
            py::tuple d(2);
            d[0] = qvariantToPyObject(QVariant::fromValue(pair.first));
            d[1] = qvariantToPyObject(QVariant::fromValue(pair.second));
            return d;
        };
        addFromQVariantFunc(newId, std::move(h));
        return newId;
    }

} // namespace qtpyt
