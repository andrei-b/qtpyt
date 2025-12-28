#pragma once
#define PYBIND11_NO_KEYWORDS

#include <pybind11/pybind11.h>
#include <QVariant>

namespace py = pybind11;

namespace  qtpyt {
    using PyObjectFromQVariantFunc = std::function<py::object(const QVariant&)>;
    std::optional<QVariant> pyObjectToQVariant(const py::handle& obj, const QByteArray& expectedType = {});
    QVariantList pySequenceToVariantList(const py::iterable& seq);
    QVariantMap pyDictToVariantMap(const py::dict& d);

    using ValueFromStringFunc = std::function<QVariant(const QString&)>;
    using ValueFromSequenceFunc = std::function<QVariant(py::sequence&)>;
    using ValueFromDictFunc =  std::function<QVariant(py::dict&)>;
    using PyObjectFromVoidPtrFunc = std::function<py::object(const void*)>;
    using QVariantFromPyObjectFunc = std::function<QVariant(const py::object&)>;

    void addFromSequenceFunc(const QString& typeName, ValueFromSequenceFunc &&func);
    void addMetatypeVoidPtrToPyObjectConverterFunc(QMetaType::Type type, PyObjectFromVoidPtrFunc&& func);
    void addSpecializedMetatypeConverter(int typeId, QVariantFromPyObjectFunc&& func);

    template<typename Container>
    void addSequenceFromSequenceConverter(const QString& typeName) {
        using T = typename Container::value_type;
        ValueFromSequenceFunc f = [](py::sequence& seq) -> QVariant {
            Container c;
            for (auto item : seq) {
                // use std::inserter so both sequence and associative containers work
                *std::inserter(c, c.end()) = item.cast<T>();
            }
            return QVariant::fromValue(c);
        };
        addFromSequenceFunc(typeName, std::move(f));
    }



    template<typename T>
    struct is_pair_like : std::false_type {};

    template<typename A, typename B>
    struct is_pair_like<std::pair<A, B>> : std::true_type {};



    py::object qvariantToPyObject(const QVariant& var);

    py::object qmetatypeToPyObject(int typeId, const void* data);

    void addFromQVariantFunc(int typeId, PyObjectFromQVariantFunc &&func);

    template<typename Container>
    void addSequenceToPyListConverter(int typeId) {
        using T = typename Container::value_type;
        PyObjectFromQVariantFunc f = [](const QVariant& v) {
            const auto container = v.template value<Container>();
            py::list t;
            for (const auto &elem : container) t.append(py::cast(elem));
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
        (is_pair_like<container_value_t<Container>>::value &&
         std::is_same_v<typename container_value_t<Container>::second_type, QVariant>);

    template<typename Container>
    void addAssociativeToPyDictConverter(int typeId) {
        using value_type = typename Container::value_type;

        static_assert(is_pair_like<value_type>::value, "addAssociativeToPyDictConverter requires a pair-like Container::value_type");

        PyObjectFromQVariantFunc f = [](const QVariant& v) {
            const auto container = v.template value<Container>();
            py::dict d;
            for (const auto &p : container) {
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
        PyObjectFromQVariantFunc f = [](const QVariant& v) {
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
    int registerContainerType(const QString& name) {
        auto newId = qRegisterMetaType<Container>(name.toStdString().c_str());
        addSequenceFromSequenceConverter<Container>(name);
        if constexpr (is_pair_like<typename Container::value_type>::value) {
            addAssociativeToPyDictConverter<Container>(newId);
        } else {
            addSequenceToPyTupleConverter<Container>(newId);
        }
        return newId;
    }

    std::optional<QVariant> pyObjectToQVariant(const py::handle& obj, int typeId);

    QVector<int> parameterTypeIds(const QList<QByteArray>& paramTypes);

    void addFromPyObjectToQVariantFunc(const QString& name, QVariantFromPyObjectFunc&& func);
} // namespace qtpyt