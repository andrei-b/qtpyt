
#include "conversions.h"
#include <QByteArray>
#include <QColor>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMatrix4x4>
#include <QPoint>
#include <QPointF>
#include <QQuaternion>
#include <QRect>
#include <QRectF>
#include <QSize>
#include <QSizeF>
#include <QStringList>
#include <QUrl>
#include <QUuid>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <pybind11/numpy.h>
#include <qkeysequence.h>
#include "internal/normalize.h"


namespace qtpyt {

    static inline QByteArray makeNormalName(const QByteArray& name) {
        auto nn = QByteArray(QMetaType::fromName(name).name());
        if (nn.isEmpty()) {
            return name;
        }
        return nn;
    }

    auto tupleFromQSize(const QVariant& v) {
        const auto s = v.toSize();
        py::tuple t(2);
        t[0] = py::int_(s.width());
        t[1] = py::int_(s.height());
        return t;
    }

    py::memoryview qByteArrayToPyArray(const QVariant& ba) {
        auto keeper = new QByteArray(ba.value<QByteArray>());
        py::capsule cap(keeper, [](void* p) { delete static_cast<QByteArray*>(p); });

        // Export a buffer whose "exporter object" is the capsule (so refcount keeps it alive)
        Py_buffer view{};
        PyObject* exporter = cap.ptr();
        Py_INCREF(exporter); // PyBuffer_FillInfo will store exporter in view.obj

        // readonly=1, flags=contiguous
        if (PyBuffer_FillInfo(&view, exporter, (void*)keeper->constData(), (Py_ssize_t)keeper->size(),
                              /*readonly=*/1, PyBUF_CONTIG_RO) != 0) {
            Py_DECREF(exporter);
            throw py::error_already_set();
                              }

        // memoryview takes ownership of the Py_buffer (it will DECREF view.obj when done)
        PyObject* mv = PyMemoryView_FromBuffer(&view);
        if (!mv) {
            // If creation failed, clean up exporter ref
            PyBuffer_Release(&view);
            throw py::error_already_set();
        }

        return py::reinterpret_steal<py::memoryview>(mv);
    }

    auto tupleFromQPoint(const QVariant& v) {
        const auto s = v.toPoint();
        py::tuple t(2);
        t[0] = py::int_(s.x());
        t[1] = py::int_(s.y());
        return t;
    }

    auto tupleFromQRect(const QVariant& v) {
        const auto r = v.toRect();
        py::tuple t(4);
        t[0] = py::int_(r.x());
        t[1] = py::int_(r.y());
        t[2] = py::int_(r.width());
        t[3] = py::int_(r.height());
        return t;
    }

    auto tupleFromQPointF(const QVariant& v) {
        const auto p = v.toPointF();
        py::tuple t(2);
        t[0] = py::float_(p.x());
        t[1] = py::float_(p.y());
        return t;
    }

    auto tupleFromQSizeF(const QVariant& v) {
        const auto s = v.toSizeF();
        py::tuple t(2);
        t[0] = py::float_(s.width());
        t[1] = py::float_(s.height());
        return t;
    }

    auto tupleFromQRectF(const QVariant& v) {
        const auto r = v.toRectF();
        py::tuple t(4);
        t[0] = py::float_(r.x());
        t[1] = py::float_(r.y());
        t[2] = py::float_(r.width());
        t[3] = py::float_(r.height());
        return t;
    }

    auto tupleFromQColor(const QVariant& v) {
        const auto c = v.value<QColor>();
        py::tuple t(4);
        t[0] = py::int_(c.red());
        t[1] = py::int_(c.green());
        t[2] = py::int_(c.blue());
        t[3] = py::int_(c.alpha());
        return t;
    }

    auto bytesFromQByteArray(const QVariant& v) {
        const auto ba = v.toByteArray();
        return py::bytes(ba.constData(), static_cast<ssize_t>(ba.size()));
    }

    auto listFromStringList(const QVariant& v) {
        const auto list = v.toStringList();
        py::list pylist;
        for (const QString& str : list) {
            pylist.append(py::str(str.toStdString()));
        }
        return pylist;
    }

    auto tupleFromQVector2D(const QVariant& v) {
        if (v.canConvert<QVector2D>()) {
            const auto vec = v.value<QVector2D>();
            py::tuple tuple(2);
            tuple[0] = static_cast<double>(vec.x());
            tuple[1] = static_cast<double>(vec.y());
            return tuple;
        }
        throw std::runtime_error("argument cannot be converted to QVector2D");
    }

    auto tupleFromQVector3D(const QVariant& v) {
        if (v.canConvert<QVector3D>()) {
            const auto vec = v.value<QVector3D>();
            py::tuple tuple(3);
            tuple[0] = static_cast<double>(vec.x());
            tuple[1] = static_cast<double>(vec.y());
            tuple[2] = static_cast<double>(vec.z());
            return tuple;
        }
        throw std::runtime_error("argument cannot be converted to QVector3D");
    }

    auto tupleFromQVector4D(const QVariant& v) {
        if (v.canConvert<QVector4D>()) {
            const auto vec = v.value<QVector4D>();
            py::tuple tuple(4);
            tuple[0] = static_cast<double>(vec.x());
            tuple[1] = static_cast<double>(vec.y());
            tuple[2] = static_cast<double>(vec.z());
            tuple[3] = static_cast<double>(vec.w());
            return tuple;
        }
        throw std::runtime_error("argument cannot be converted to QVector4D");
    }

    auto tupleFromQuaternion(const QVariant& v) {
        if (v.canConvert<QQuaternion>()) {
            const auto quat = v.value<QQuaternion>();
            py::tuple tuple(4);
            tuple[0] = static_cast<double>(quat.x());
            tuple[1] = static_cast<double>(quat.y());
            tuple[2] = static_cast<double>(quat.z());
            tuple[3] = static_cast<double>(quat.scalar());
            return tuple;
        }
        throw std::runtime_error("argument cannot be converted to QQuaternion");
    }

    auto tupleFromMatrix4x4(const QVariant& v) {
        if (v.canConvert<QMatrix4x4>()) {
            const auto mat = v.value<QMatrix4x4>();
            py::tuple tuple(4);
            for (int i = 0; i < 4; ++i) {
                py::tuple row(4);
                for (int j = 0; j < 4; ++j) {
                    row[j] = static_cast<double>(mat(i, j));
                }
                tuple[i] = row;
            }
            return tuple;
        }
        throw std::runtime_error("argument cannot be converted to QMatrix4x4");
    }

    static std::unordered_map<int, PyObjectFromQVariantFunc> specializedQVariantToPyObjectConverters = {
        {QMetaType::Int, [](const QVariant& v) { return py::int_(v.toInt()); }},
        {QMetaType::Double, [](const QVariant& v) { return py::float_(v.toDouble()); }},
        {QMetaType::QString, [](const QVariant& v) { return py::str(v.toString().toStdString()); }},
        {QMetaType::UInt, [](const QVariant& v) { return py::int_(v.toUInt()); }},
        {QMetaType::LongLong, [](const QVariant& v) { return py::int_(v.toLongLong()); }},
        {QMetaType::ULongLong, [](const QVariant& v) { return py::int_(v.toULongLong()); }},
        {QMetaType::Long, [](const QVariant& v) { return py::int_(v.toInt()); }},
        {QMetaType::ULong, [](const QVariant& v) { return py::int_(v.toInt()); }},
        {QMetaType::Short, [](const QVariant& v) { return py::int_(v.toInt()); }},
        {QMetaType::UShort, [](const QVariant& v) { return py::int_(v.toInt()); }},
        {QMetaType::Bool, [](const QVariant& v) { return py::bool_(v.toBool()); }},
        {QMetaType::Float, [](const QVariant& v) { return py::float_(v.toFloat()); }},
        {QMetaType::Char,
         [](const QVariant& v) {
             const QChar qc = v.toChar();
             return py::str(std::string(1, qc.toLatin1()));
        }},
       {QMetaType::UChar, [](const QVariant& v) { return py::int_(v.toUInt()); }},
       {QMetaType::VoidStar,
        [](const QVariant& v) {
            const void* ptr = v.value<void*>();
            return py::int_(reinterpret_cast<uintptr_t>(ptr));
       }},
      {QMetaType::QObjectStar,
       [](const QVariant& v) {
           const QObject* obj = v.value<QObject*>();
           return py::int_(reinterpret_cast<uintptr_t>(obj));
      }},
     {QMetaType::QSize, tupleFromQSize},
     {QMetaType::QPoint, tupleFromQPoint},
     {QMetaType::QRect, tupleFromQRect},
     {QMetaType::QSizeF, tupleFromQSizeF},
     {QMetaType::QPointF, tupleFromQPointF},
     {QMetaType::QRectF, tupleFromQRectF},
     {QMetaType::QColor, tupleFromQColor},
     {QMetaType::QUrl, [](const QVariant& v) { return py::str(v.toUrl().toString().toStdString()); }},
     {QMetaType::QDateTime, [](const QVariant& v) { return py::str(v.toDateTime().toString().toStdString()); }},
     {QMetaType::QDate, [](const QVariant& v) { return py::str(v.toDate().toString().toStdString()); }},
     {QMetaType::QTime, [](const QVariant& v) { return py::str(v.toTime().toString().toStdString()); }},
     {QMetaType::QByteArray, qByteArrayToPyArray},
     {QMetaType::QStringList, listFromStringList},
     {QMetaType::QVector2D, tupleFromQVector2D},
     {QMetaType::QVector3D, tupleFromQVector3D},
     {QMetaType::QVector4D, tupleFromQVector4D},
     {QMetaType::QQuaternion, tupleFromQuaternion},
     {QMetaType::QMatrix4x4, tupleFromMatrix4x4},
     {QMetaType::QUuid, [](const QVariant& v) { return py::str(v.toUuid().toString().toStdString()); }}};

    void addFromQVariantFunc(int typeId, PyObjectFromQVariantFunc&& func) {
        specializedQVariantToPyObjectConverters.insert({typeId, std::move(func)});
    }

    py::object qvariantToPyObject(const QVariant& var) {

        const auto tid = var.typeId();
        if (specializedQVariantToPyObjectConverters.contains(tid)) {
            return specializedQVariantToPyObjectConverters.at(tid)(var);
        }
        switch (tid) {
            case QMetaType::QVariant: {
                return qvariantToPyObject(var.value<QVariant>());
                break;
            }
            case QMetaType::QVariantList: {
                const auto list = var.toList();
                py::tuple tuple(std::distance(list.begin(), list.end()));
                int index = 0;
                for (const QVariant& item : list) {
                    tuple[index++] = qvariantToPyObject(item);
                }
                return tuple;
                break;
            }
            case QMetaType::QVariantMap: {
                const auto map = var.toMap();
                py::dict dict;
                for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
                    dict[py::str(qvariantToPyObject(it.key()))] = qvariantToPyObject(it.value());
                }
                return dict;
                break;
            }
            case QMetaType::QVariantHash: {
                const auto hash = var.toHash();
                py::dict dict;
                for (auto it = hash.constBegin(); it != hash.constEnd(); ++it) {
                    dict[py::str(qvariantToPyObject(it.key()))] = qvariantToPyObject(it.value());
                }
                return dict;
                break;
            }
            default:
                return py::str(var.toString().toStdString());
                break;
        }
        return py::none();
    }

    auto squenceToQPoint(py::sequence& seq) {
        if (seq.size() != 2 || !py::isinstance<py::int_>(seq[0]) || !py::isinstance<py::int_>(seq[1])) {
            throw std::runtime_error("Expected a tuple of two integers for QPoint conversion");
        }
        const int a = seq[0].cast<int>();
        const int b = seq[1].cast<int>();
        return QVariant::fromValue(QPoint(a, b));
    }

    auto squenceToQSize(py::sequence& seq) {
        if (seq.size() != 2 || !py::isinstance<py::int_>(seq[0]) || !py::isinstance<py::int_>(seq[1])) {
            throw std::runtime_error("Expected a tuple of two integers for QSize conversion");
        }
        const int a = seq[0].cast<int>();
        const int b = seq[1].cast<int>();
        return QVariant::fromValue(QSize(a, b));
    }

    auto squenceToQRect(py::sequence& seq) {
        if (seq.size() != 4 || !py::isinstance<py::int_>(seq[0]) || !py::isinstance<py::int_>(seq[1]) ||
            !py::isinstance<py::int_>(seq[2]) || !py::isinstance<py::int_>(seq[3])) {
            throw std::runtime_error("Expected a tuple of four integers for QRect conversion");
            }
        const int x = seq[0].cast<int>();
        const int y = seq[1].cast<int>();
        const int w = seq[2].cast<int>();
        const int h = seq[3].cast<int>();
        return QVariant::fromValue(QRect(x, y, w, h));
    }

    auto squenceToQColor(py::sequence& seq) {
        if (seq.size() == 3) {
            if (!py::isinstance<py::int_>(seq[0]) || !py::isinstance<py::int_>(seq[1]) ||
                !py::isinstance<py::int_>(seq[2])) {
                throw std::runtime_error("Expected a tuple of three integers for QColor conversion");
                }
            const int r = seq[0].cast<int>();
            const int g = seq[1].cast<int>();
            const int b = seq[2].cast<int>();
            return QVariant(QColor(r, g, b));
        }
        if (seq.size() == 4) {
            if (!py::isinstance<py::int_>(seq[0]) || !py::isinstance<py::int_>(seq[1]) ||
                !py::isinstance<py::int_>(seq[2]) || !py::isinstance<py::int_>(seq[3])) {
                throw std::runtime_error("Expected a tuple of four integers for QColor conversion");
                }
            const int r = seq[0].cast<int>();
            const int g = seq[1].cast<int>();
            const int b = seq[2].cast<int>();
            const int a = seq[3].cast<int>();
            return QVariant(QColor(r, g, b, a));
        }
        throw std::runtime_error("Expected a tuple of 3 or 4 values for QColor conversion");
    }

    auto tupleToQPointF(py::sequence& seq) {
        if (seq.size() != 2 || !py::isinstance<py::float_>(seq[0]) || !py::isinstance<py::float_>(seq[1])) {
            throw std::runtime_error("Expected a tuple of two floats for QPointF conversion");
        }
        const double a = seq[0].cast<double>();
        const double b = seq[1].cast<double>();
        return QVariant::fromValue(QPointF(a, b));
    }

    auto tupleToQSizeF(py::sequence& seq) {
        if (seq.size() != 2 || !py::isinstance<py::float_>(seq[0]) || !py::isinstance<py::float_>(seq[1])) {
            throw std::runtime_error("Expected a tuple of two floats for QSizeF conversion");
        }
        const double a = seq[0].cast<double>();
        const double b = seq[1].cast<double>();
        return QVariant::fromValue(QSizeF(a, b));
    }

    auto tupleToQRectF(py::sequence& seq) {
        if (seq.size() != 4 || !py::isinstance<py::float_>(seq[0]) || !py::isinstance<py::float_>(seq[1]) ||
            !py::isinstance<py::float_>(seq[2]) || !py::isinstance<py::float_>(seq[3])) {
            throw std::runtime_error("Expected a tuple of four floats for QRectF conversion");
            }
        const double x = seq[0].cast<double>();
        const double y = seq[1].cast<double>();
        const double w = seq[2].cast<double>();
        const double h = seq[3].cast<double>();
        return QVariant::fromValue(QRectF(x, y, w, h));
    }

    auto sequenceToVector(py::sequence& seq) {
        switch (seq.size()) {
            case 2: {
                const double a = seq[0].cast<double>();
                const double b = seq[1].cast<double>();
                return QVariant::fromValue(QVector2D(a, b));
            }
            case 3: {
                const double a = seq[0].cast<double>();
                const double b = seq[1].cast<double>();
                const double c = seq[2].cast<double>();
                return QVariant::fromValue(QVector3D(a, b, c));
            }
            case 4: {
                const double a = seq[0].cast<double>();
                const double b = seq[1].cast<double>();
                const double c = seq[2].cast<double>();
                const double d = seq[3].cast<double>();
                return QVariant::fromValue(QVector4D(a, b, c, d));
            }
            default:
                throw std::runtime_error("Expected a tuple of 2, 3, or 4 floats for QVector conversion");
        }
    }

    auto sequenceToQQuaternion(py::sequence& seq) {
        if (seq.size() != 4 || !py::isinstance<py::float_>(seq[0]) || !py::isinstance<py::float_>(seq[1]) ||
            !py::isinstance<py::float_>(seq[2]) || !py::isinstance<py::float_>(seq[3])) {
            throw std::runtime_error("Expected a tuple of four floats for QQuaternion conversion");
            }
        const double x = seq[0].cast<double>();
        const double y = seq[1].cast<double>();
        const double z = seq[2].cast<double>();
        const double w = seq[3].cast<double>();
        return QVariant::fromValue(QQuaternion(x, y, z, w));
    }

    auto sequenceToMatrix4x4(py::sequence& seq) {
        if (seq.size() != 16) {
            throw std::runtime_error("Expected a tuple of sixteen floats for QMatrix4x4 conversion");
        }
        QMatrix4x4 mat;
        for (int i = 0; i < 16; ++i) {
            if (!py::isinstance<py::float_>(seq[i])) {
                throw std::runtime_error("Expected all elements to be floats for QMatrix4x4 conversion");
            }
            mat.data()[i] = seq[i].cast<float>();
        }
        return QVariant::fromValue(mat);
    }

    auto sequenceToQVariantList(const py::object& obj) {
        if (!obj || obj.is_none()) {
            return QVariantList();
        }
        if (py::isinstance<py::list>(obj) || py::isinstance<py::tuple>(obj)) {
            QVariantList list;
            auto seq = py::reinterpret_borrow<py::sequence>(obj);
            for (auto item : seq) {
                auto v = pyObjectToQVariant(item);
                if (!v.has_value()) {
                    throw std::runtime_error("sequenceToQVariantList: Failed to convert item to QVariant in QVariantList conversion");
                }
                list.append(v.value());
            }
            return list;
        }
        throw std::runtime_error("Expected a list or tuple for QVariantList conversion");
    }

    auto dictToQVariantMap(const py::object& obj) {
        if (!obj || obj.is_none()) {
            return QVariantMap();
        }
        if (py::isinstance<py::dict>(obj)) {
            QVariantMap map;
            auto dict = py::reinterpret_borrow<py::dict>(obj);
            auto keyName = makeNormalName("QString");
            auto valueName = makeNormalName("QVariant");
            for (auto item : dict) {
                const auto key = pyObjectToQVariant(item.first, keyName).value().toString();
                const auto value = pyObjectToQVariant(item.second, valueName);
                if (!value.has_value()) {
                    throw std::runtime_error("dictToQVariantMap: Failed to convert value to QVariant in QVariantMap conversion");
                }
                map.insert(key, value.value());
            }
            return map;
        }
        throw std::runtime_error("Expected a dict for QVariantMap conversion");
    }

    auto dictToQVariantHash(const py::object& obj) {
        if (!obj || obj.is_none()) {
            return QVariantHash();
        }
        if (py::isinstance<py::dict>(obj)) {
            QVariantHash hash;
            auto dict = py::reinterpret_borrow<py::dict>(obj);
            for (auto item : dict) {
                const auto key = item.first.cast<QString>();
                const auto value = pyObjectToQVariant(item.second);
                if (!value.has_value()) {
                    throw std::runtime_error("dictToQVariantHash: Failed to convert value to QVariant in QVariantHash conversion");
                } else {
                    hash.insert(key, value.value());
                }
            }
            return hash;
        }
        throw std::runtime_error("Expected a dict for QVariantHash conversion");
    }

    auto byteArrayToQVariant(const py::object& obj) {
        if (!obj || obj.is_none()) {
            return QVariant();
        }
        if (py::isinstance<py::bytes>(obj)) {
            const auto b = obj.cast<std::string>();
            return QVariant::fromValue(QByteArray(b.data(), static_cast<int>(b.size())));
        }
        throw std::runtime_error("Expected bytes object for QByteArray conversion");
    }

    auto stringListToQVariant(const py::object& obj) {
        if (!obj || obj.is_none()) {
            return QVariant();
        }
        if (py::isinstance<py::list>(obj) || py::isinstance<py::tuple>(obj)) {
            QVariantList list;
            auto seq = py::reinterpret_borrow<py::sequence>(obj);
            for (auto item : seq) {
                if (!py::isinstance<py::str>(item)) {
                    throw std::runtime_error("Expected a list or tuple of strings for QStringList conversion");
                }
                const auto str = QString::fromStdString(item.cast<std::string>());
                list.append(str);
            }
            return QVariant::fromValue(list);
        }
        throw std::runtime_error("Expected a list or tuple for QStringList conversion");
    }

    const static std::unordered_map<QByteArray, ValueFromStringFunc> specializedStringConverters = {
        {makeNormalName("QUrl"), [](const QString& val) { return QVariant::fromValue(QUrl(val)); }},
        {makeNormalName("QTime"), [](const QString& val) { return QVariant::fromValue(QTime::fromString(val)); }},
        {makeNormalName("QDate"), [](const QString& val) { return QVariant::fromValue(QDate::fromString(val)); }},
        {makeNormalName("QDateTime"), [](const QString& val) { return QVariant::fromValue(QDateTime::fromString(val)); }},
        {makeNormalName("QColor"), [](const QString& val) { return QVariant(QColor(val)); }},
        {makeNormalName("QByteArray"),
         [](const QString& val) { return QVariant::fromValue(QByteArray::fromStdString(val.toStdString())); }},
        {makeNormalName("QUuid"), [](const QString& val) { return QVariant::fromValue(QUuid(val)); }}};

    static std::unordered_map<QString, ValueFromSequenceFunc> specializedSequenceConverters {
        {makeNormalName("QByteArray"), byteArrayToQVariant},
        {makeNormalName("QStringList"), stringListToQVariant},
        {makeNormalName("QPoint"), squenceToQPoint},
        {makeNormalName("QSize"), squenceToQSize},
        {makeNormalName("QRect"), squenceToQRect},
        {makeNormalName("QColor"), squenceToQColor},
        {makeNormalName("QPointF"), tupleToQPointF},
        {makeNormalName("QSizeF"), tupleToQSizeF},
        {makeNormalName("QRectF"), tupleToQRectF},
        {makeNormalName("QVector2D"), sequenceToVector},
        {makeNormalName("QVector3D"), sequenceToVector},
        {makeNormalName("QVector4D"), sequenceToVector},
        {makeNormalName("QQuaternion"), sequenceToQQuaternion},
        {makeNormalName("QMatrix4x4"), sequenceToMatrix4x4},
        {makeNormalName("QVariantList"), sequenceToQVariantList}};
    ;

    static std::unordered_map<QString, ValueFromDictFunc> specializedDictConverters {
        {makeNormalName("QVariantMap"), dictToQVariantMap}, {makeNormalName("QVariantHash"), dictToQVariantHash}};

    static std::unordered_map<QString, QVariantFromPyObjectFunc> specializedPyObjectConverters{};

    void addFromPyObjectToQVariantFunc(const QString& name, QVariantFromPyObjectFunc&& func) {
        const auto normName = makeNormalName(name.toStdString().c_str());
        specializedPyObjectConverters.insert({normName, std::move(func)});
    }

    void addFromDictFunc(const QString &name, ValueFromDictFunc &&func) {
        const auto normName = makeNormalName(name.toStdString().c_str());
        specializedDictConverters.insert({normName, std::move(func)});
    }

    static std::unordered_map<int, QVariantFromPyObjectFunc> specializedMetatypeConverters = {
        {QMetaType::QByteArray, byteArrayToQVariant},
        {QMetaType::QStringList, stringListToQVariant},
        {QMetaType::QVariantList, sequenceToQVariantList},
        {QMetaType::QVariantMap, dictToQVariantMap},
        {QMetaType::QVariantHash, dictToQVariantHash}};

    void addSpecializedMetatypeConverter(int typeId, QVariantFromPyObjectFunc&& func) {
        specializedMetatypeConverters.insert({typeId, std::move(func)});
    }

    using ValueFromPOD = std::function<QVariant(const py::handle&)>;

    const static std::unordered_map<QString, ValueFromPOD> specializedPodConverters = {
        {makeNormalName("int"),
         [](const py::handle& obj) {
             if (!py::isinstance<py::int_>(obj)) {
                 throw std::runtime_error("Expected an integer for Int conversion");
             }
             return QVariant::fromValue(obj.cast<int>());
        }},
       {makeNormalName("unsigned int"),
        [](const py::handle& obj) {
            if (!py::isinstance<py::int_>(obj)) {
                throw std::runtime_error("Expected an integer for UInt conversion");
            }
            return QVariant::fromValue(obj.cast<unsigned int>());
       }},
      {makeNormalName("long long"),
       [](const py::handle& obj) {
           if (!py::isinstance<py::int_>(obj)) {
               throw std::runtime_error("Expected an integer for LongLong conversion");
           }
           return QVariant::fromValue(obj.cast<qlonglong>());
      }},
     {makeNormalName("unsigned long long"),
      [](const py::handle& obj) {
          if (!py::isinstance<py::int_>(obj)) {
              throw std::runtime_error("Expected an integer for ULongLong conversion");
          }
          return QVariant::fromValue(obj.cast<qulonglong>());
     }},
    {makeNormalName("long"),
     [](const py::handle& obj) {
         if (!py::isinstance<py::int_>(obj)) {
             throw std::runtime_error("Expected an integer for Long conversion");
         }
         return QVariant::fromValue(obj.cast<long>());
    }},
   {makeNormalName("unsigned long"),
    [](const py::handle& obj) {
        if (!py::isinstance<py::int_>(obj)) {
            throw std::runtime_error("Expected an integer for ULong conversion");
        }
        return QVariant::fromValue(obj.cast<unsigned long>());
   }},
  {makeNormalName("short"),
   [](const py::handle& obj) {
       if (!py::isinstance<py::int_>(obj)) {
           throw std::runtime_error("Expected an integer for Short conversion");
       }
       return QVariant::fromValue(obj.cast<short>());
  }},
 {makeNormalName("unsigned short"),
  [](const py::handle& obj) {
      if (!py::isinstance<py::int_>(obj)) {
          throw std::runtime_error("Expected an integer for UShort conversion");
      }
      return QVariant::fromValue(obj.cast<unsigned short>());
 }},
{makeNormalName("float"),
 [](const py::handle& obj) {
     if (!py::isinstance<py::float_>(obj)) {
         throw std::runtime_error("Expected a float for Float conversion");
     }
     return QVariant::fromValue(obj.cast<float>());
}},
{makeNormalName("double"),
[](const py::handle& obj) {
    if (!py::isinstance<py::float_>(obj)) {
        throw std::runtime_error("Expected a float for Double conversion");
    }
    return QVariant::fromValue(obj.cast<double>());
}},
{makeNormalName("bool"),
[](const py::handle& obj) {
    if (!py::isinstance<py::bool_>(obj)) {
        throw std::runtime_error("Expected a boolean for Bool conversion");
    }
    return QVariant::fromValue(obj.cast<bool>());
}},
{makeNormalName("char"),
[](const py::handle& obj) {
    if (!py::isinstance<py::str>(obj)) {
        throw std::runtime_error("Expected a string for Char conversion");
    }
    const auto str = obj.cast<std::string>();
    if (str.size() != 1) {
        throw std::runtime_error("Expected a single character string for Char conversion");
    }
    return QVariant::fromValue(QChar(str[0]));
}},
{makeNormalName("unsigned char"),
[](const py::handle& obj) {
    if (!py::isinstance<py::int_>(obj)) {
        throw std::runtime_error("Expected an integer for UChar conversion");
    }
    const int val = obj.cast<int>();
    if (val < 0 || val > 255) {
        throw std::runtime_error("Expected an integer between 0 and 255 for UChar conversion");
    }
    return QVariant::fromValue(static_cast<uchar>(val));
}},
{makeNormalName("void *"),
[](const py::handle& obj) {
    if (!py::isinstance<py::int_>(obj)) {
        throw std::runtime_error("Expected an integer for VoidStar conversion");
    }
    const auto ptr = obj.cast<uintptr_t>();
    return QVariant::fromValue(reinterpret_cast<void*>(ptr));
}},
{makeNormalName("QObject *"), [](const py::handle& obj) {
    if (!py::isinstance<py::int_>(obj)) {
        throw std::runtime_error("Expected an integer for QObject* conversion");
    }
    const auto ptr = obj.cast<uintptr_t>();
    auto q = reinterpret_cast<QObject*>(ptr);
    return QVariant::fromValue(q);
}}};

    std::optional<QVariant> pyObjectToQVariant(const py::handle& obj, const QByteArray& expectedType) {
        if (!obj || obj.is_none()) {
            return std::nullopt;
        }
        if (!expectedType.isEmpty()) {
            const auto normName = makeNormalName(expectedType);
            if (specializedPyObjectConverters.contains(normName)) {
                return specializedPyObjectConverters.at(normName)(static_cast<const pybind11::object&>(obj));
            }
            if (py::isinstance<py::str>(obj)) {
                const auto val = QString::fromStdString(obj.cast<std::string>());
                if (specializedStringConverters.contains(normName)) {
                    return specializedStringConverters.at(normName)(val);
                }
            }

            if (py::isinstance<py::dict>(obj)) {
                auto dict = py::reinterpret_borrow<py::dict>(obj);
                if (specializedDictConverters.contains(normName)) {
                    return specializedDictConverters.at(normName)(dict);
                }
            }

            if (py::isinstance<py::list>(obj) || py::isinstance<py::tuple>(obj)) {
                auto seq = py::reinterpret_borrow<py::sequence>(obj);
                if (specializedSequenceConverters.contains(normName)) {
                    auto f = specializedSequenceConverters.at(normName);
                    return f(seq);
                }
            }

            if (specializedPodConverters.contains(normName)) {
                return specializedPodConverters.at(normName)(obj);
            }
        }

        if (py::isinstance<py::bool_>(obj)) {
            return QVariant::fromValue(obj.cast<bool>());
        }

        if (py::isinstance<py::int_>(obj)) {
            return QVariant::fromValue(qint64(obj.cast<long long>()));
        }

        if (py::isinstance<py::float_>(obj)) {
            return QVariant::fromValue(obj.cast<double>());
        }

        if (py::isinstance<py::str>(obj)) {
            return QVariant::fromValue(QString::fromStdString(obj.cast<std::string>()));
        }

        if (py::isinstance<py::bytes>(obj)) {
            const auto b = obj.cast<std::string>();
            return QVariant::fromValue(QByteArray(b.data(), static_cast<int>(b.size())));
        }

        if (py::isinstance<py::list>(obj) || py::isinstance<py::tuple>(obj)) {
            return QVariant::fromValue(pySequenceToVariantList(py::reinterpret_borrow<py::iterable>(obj)));
        }

        if (py::isinstance<py::dict>(obj)) {
            return QVariant::fromValue(pyDictToVariantMap(obj.cast<py::dict>()));
        }

        if (py::isinstance<py::memoryview>(obj) && makeNormalName(expectedType) == makeNormalName("QByteArray")) {
            auto memoryviewDataPtr = [](const py::memoryview& mv, size_t& outBytes) -> const char* {
                if (!mv || mv.is_none()) throw std::runtime_error("memoryview is null");
                py::buffer_info info =  py::buffer(mv).request();
                outBytes = static_cast<size_t>(info.size) * info.itemsize;
                return static_cast<const char*>(info.ptr);
            };
            py::memoryview mv = obj.cast<py::memoryview>();
            size_t nbytes;
            auto ptr = memoryviewDataPtr(mv, nbytes);
            return QVariant::fromValue(QByteArray(ptr, static_cast<int>(nbytes)));
        }

        if (expectedType.isEmpty() && (expectedType == "QObject" || expectedType == "QObject*") &&
            py::isinstance<py::int_>(obj)) {

            const uintptr_t ptr = obj.cast<uintptr_t>();
            if (auto q = reinterpret_cast<QObject*>(ptr)) {
                return QVariant::fromValue(qintptr(reinterpret_cast<qintptr>(q)));
            }
            }

        try {
            std::string s = py::str(obj);
            return QVariant::fromValue(QString::fromStdString(s));
        } catch (...) {
            return std::nullopt;
        }
    }

    std::optional<QVariant> pyObjectToQVariant(const py::handle& obj, int typeId) {
        if (!obj || obj.is_none()) {
            return std::nullopt;
        }
        if (specializedMetatypeConverters.contains(typeId)) {
            return specializedMetatypeConverters.at(typeId)(static_cast<const pybind11::object&>(obj));
        }
        return std::nullopt;
    }

    QVector<int> parameterTypeIds(const QList<QByteArray>& paramTypes) {
        QVector<int> ids;
        ids.reserve(paramTypes.size());

        for (const QByteArray& raw : paramTypes) {
            // Normalize name: remove "const", '&', '*' and trim
            QString name = QString::fromUtf8(raw).trimmed();
            name.replace(QStringLiteral("const"), QString()); // remove "const"
            name.remove(QChar('&'));
            name.remove(QChar('*'));
            name = name.trimmed();

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            // Qt6: use QMetaType::fromName(...)
            QMetaType mt = QMetaType::fromName(name.toUtf8());
            ids.append(mt.id());
#else
            // Qt5: use QMetaType::type(...)
            ids.append(QMetaType::type(name.toUtf8().constData()));
#endif
        }

        return ids;
    }

    QVariantList pySequenceToVariantList(const py::iterable& seq) {
        QVariantList out;
        for (auto item : seq) {
            auto v = pyObjectToQVariant(item);
            if (!v.has_value()) {
                throw std::runtime_error("pySequenceToVariantList: Failed to convert item to QVariant in QVariantList conversion");
            }
            out.push_back(v.value());
        }
        return out;
    }

    QVariantMap pyDictToVariantMap(const py::dict& d) {
        QVariantMap map;
        for (auto [fst, snd] : d) {
            auto key = fst.cast<std::string>();
            auto v = pyObjectToQVariant(snd);
            if (!v.has_value()) {
                throw std::runtime_error("pyDictToVariantMap: Failed to convert value to QVariant in QVariantMap conversion");
            }
            map[QString::fromStdString(key)] = v.value();;
        }
        return map;
    }
    void addFromSequenceFunc(const QString& typeName, ValueFromSequenceFunc&& func) {
        const auto normName = makeNormalName(typeName.toLatin1());
        specializedSequenceConverters.insert({normName, std::move(func)});
    }

    py::object voidPtrToPyObject(const void* v) {
        return py::int_(reinterpret_cast<uintptr_t>(v));
    }

    template <typename T> py::object podToPyObject(const void* v) {
        static_assert(std::is_trivial_v<T>, "T must be a POD type");
        const T val = *static_cast<const T*>(v);
        return py::cast(val);
    }

    py::memoryview qByteArrayToMemoryView(const QByteArray& ba) {
        const auto keeper = new QByteArray(ba);
        py::capsule cap(keeper, [](void* p) { delete static_cast<QByteArray*>(p); });

        // Export a buffer whose "exporter object" is the capsule (so refcount keeps it alive)
        Py_buffer view{};
        PyObject* exporter = cap.ptr();
        Py_INCREF(exporter); // PyBuffer_FillInfo will store exporter in view.obj

        // readonly=1, flags=contiguous
        if (PyBuffer_FillInfo(&view, exporter, (void*)keeper->constData(), (Py_ssize_t)keeper->size(),
                              /*readonly=*/1, PyBUF_CONTIG_RO) != 0) {
            Py_DECREF(exporter);
            throw py::error_already_set();
                              }

        // memoryview takes ownership of the Py_buffer (it will DECREF view.obj when done)
        PyObject* mv = PyMemoryView_FromBuffer(&view);
        if (!mv) {
            // If creation failed, clean up exporter ref
            PyBuffer_Release(&view);
            throw py::error_already_set();
        }

        return py::reinterpret_steal<py::memoryview>(mv);
    }

    py::object memoryViewFromQByteaArrayAsVoidPtr(const void* v) {
        QByteArray ba = *static_cast<const QByteArray*>(v);
        return qByteArrayToMemoryView(ba);
    }

    template <typename T> py::object podArrayToPyArray(const void* data, size_t count) {
        auto keeper = new std::vector<T>(count);
        std::memcpy(keeper->data(), data, sizeof(T) * count);
        py::capsule cap(keeper, [](void* p) { delete static_cast<std::vector<T>*>(p); });

        // Export a buffer whose "exporter object" is the capsule (so refcount keeps it alive)
        Py_buffer view{};
        PyObject* exporter = cap.ptr();
        Py_INCREF(exporter); // PyBuffer_FillInfo will store exporter in view.obj

        // readonly=1, flags=contiguous
        if (PyBuffer_FillInfo(&view, exporter, (void*)keeper->data(), (Py_ssize_t)(sizeof(T) * count),
                              /*readonly=*/1, PyBUF_CONTIG_RO) != 0) {
            Py_DECREF(exporter);
            throw py::error_already_set();
                              }

        // memoryview takes ownership of the Py_buffer (it will DECREF view.obj when done)
        PyObject* mv = PyMemoryView_FromBuffer(&view);
        if (!mv) {
            // If creation failed, clean up exporter ref
            PyBuffer_Release(&view);
            throw py::error_already_set();
        }

        return py::reinterpret_steal<py::object>(mv);
    }

    // C++
    template <typename T> py::object podVectorToMemoryView_move(std::vector<T>&& vec) {
        static_assert(std::is_trivial_v<T>, "T must be a POD type");
        // Move the input into a heap allocation and let the capsule delete it.
        auto keeper = new std::vector<T>(std::move(vec));
        py::capsule cap(keeper, [](void* p) { delete static_cast<std::vector<T>*>(p); });

        Py_buffer view{};
        PyObject* exporter = cap.ptr();
        Py_INCREF(exporter);

        if (PyBuffer_FillInfo(&view, exporter, (void*)keeper->data(), (Py_ssize_t)(sizeof(T) * keeper->size()),
                              /*readonly=*/1, PyBUF_CONTIG_RO) != 0) {
            Py_DECREF(exporter);
            throw py::error_already_set();
                              }

        PyObject* mv = PyMemoryView_FromBuffer(&view);
        if (!mv) {
            PyBuffer_Release(&view);
            throw py::error_already_set();
        }

        return py::reinterpret_steal<py::object>(mv);
    }

    template <typename T> py::object podVectorToMemoryView_shared(const std::vector<T>& src) {
        static_assert(std::is_trivial_v<T>, "T must be a POD type");

        auto sp = std::make_shared<std::vector<T>>(src); // copy; if you can move, accept rvalue and use std::move(src)
        // Store the shared_ptr on the heap so the capsule can own it.
        auto ctx = new std::shared_ptr<std::vector<T>>(sp);
        py::capsule cap(ctx, [](void* p) { delete static_cast<std::shared_ptr<std::vector<T>>*>(p); });

        Py_buffer view{};
        PyObject* exporter = cap.ptr();
        Py_INCREF(exporter);

        if (PyBuffer_FillInfo(&view, exporter, (void*)sp->data(), (Py_ssize_t)(sizeof(T) * sp->size()),
                              /*readonly=*/1, PyBUF_CONTIG_RO) != 0) {
            Py_DECREF(exporter);
            throw py::error_already_set();
                              }

        PyObject* mv = PyMemoryView_FromBuffer(&view);
        if (!mv) {
            PyBuffer_Release(&view);
            throw py::error_already_set();
        }

        return py::reinterpret_steal<py::object>(mv);
    }

    py::object qpointFromVoidPtr(const void* v) {
        const QPoint point = *static_cast<const QPoint*>(v);
        py::tuple tuple(2);
        tuple[0] = point.x();
        tuple[1] = point.y();
        return tuple;
    }

    py::object qsizeFromVoidPtr(const void* v) {
        const QSize size = *static_cast<const QSize*>(v);
        py::tuple tuple(2);
        tuple[0] = size.width();
        tuple[1] = size.height();
        return tuple;
    }

    py::object qrectFromVoidPtr(const void* v) {
        const QRect rect = *static_cast<const QRect*>(v);
        py::tuple tuple(4);
        tuple[0] = rect.x();
        tuple[1] = rect.y();
        tuple[2] = rect.width();
        tuple[3] = rect.height();
        return tuple;
    }

    py::object qcolorFromVoidPtr(const void* v) {
        const QColor color = *static_cast<const QColor*>(v);
        py::tuple tuple(4);
        tuple[0] = color.red();
        tuple[1] = color.green();
        tuple[2] = color.blue();
        tuple[3] = color.alpha();
        return tuple;
    }

    py::object qdatetimeFromVoidPtr(const void* v) {
        const QDateTime dt = *static_cast<const QDateTime*>(v);
        return py::str(dt.toString().toStdString());
    }

    py::object qdateFromVoidPtr(const void* v) {
        const QDate d = *static_cast<const QDate*>(v);
        return py::str(d.toString().toStdString());
    }

    py::object qtimeFromVoidPtr(const void* v) {
        const QTime t = *static_cast<const QTime*>(v);
        return py::str(t.toString().toStdString());
    }

    py::object quuidFromVoidPtr(const void* v) {
        const QUuid u = *static_cast<const QUuid*>(v);
        return py::str(u.toString().toStdString());
    }

    py::object qurlFromVoidPtr(const void* v) {
        const QUrl url = *static_cast<const QUrl*>(v);
        return py::str(url.toString().toStdString());
    }

    py::object qstringFromVoidPtr(const void* v) {
        const QString str = *static_cast<const QString*>(v);
        return py::str(str.toStdString());
    }

    py::object qpointfromVoidPtr(const void* v) {
        const QPointF point = *static_cast<const QPointF*>(v);
        py::tuple tuple(2);
        tuple[0] = point.x();
        tuple[1] = point.y();
        return tuple;
    }

    py::object qsizefFromVoidPtr(const void* v) {
        const QSizeF size = *static_cast<const QSizeF*>(v);
        py::tuple tuple(2);
        tuple[0] = size.width();
        tuple[1] = size.height();
        return tuple;
    }

    py::object qrectfFromVoidPtr(const void* v) {
        const QRectF rect = *static_cast<const QRectF*>(v);
        py::tuple tuple(4);
        tuple[0] = rect.x();
        tuple[1] = rect.y();
        tuple[2] = rect.width();
        tuple[3] = rect.height();
        return tuple;
    }

    py::object qvector2dFromVoidPtr(const void* v) {
        const QVector2D vec = *static_cast<const QVector2D*>(v);
        py::tuple tuple(2);
        tuple[0] = vec.x();
        tuple[1] = vec.y();
        return tuple;
    }

    py::object qvector3dFromVoidPtr(const void* v) {
        const QVector3D vec = *static_cast<const QVector3D*>(v);
        py::tuple tuple(3);
        tuple[0] = vec.x();
        tuple[1] = vec.y();
        tuple[2] = vec.z();
        return tuple;
    }

    py::object qvector4dFromVoidPtr(const void* v) {
        const QVector4D vec = *static_cast<const QVector4D*>(v);
        py::tuple tuple(4);
        tuple[0] = vec.x();
        tuple[1] = vec.y();
        tuple[2] = vec.z();
        tuple[3] = vec.w();
        return tuple;
    }

    py::object quaternionFromVoidPtr(const void* v) {
        const QQuaternion quat = *static_cast<const QQuaternion*>(v);
        py::tuple tuple(4);
        tuple[0] = quat.x();
        tuple[1] = quat.y();
        tuple[2] = quat.z();
        tuple[3] = quat.scalar();
        return tuple;
    }

    py::object matrix4x4FromVoidPtr(const void* v) {
        const QMatrix4x4 mat = *static_cast<const QMatrix4x4*>(v);
        py::tuple tuple(16);
        const float* data = mat.constData();
        for (int i = 0; i < 16; ++i) {
            tuple[i] = data[i];
        }
        return tuple;
    }

    py::object qcharFromVoidPtr(const void* v) {
        const QChar ch = *static_cast<const QChar*>(v);
        std::string s;
        s += ch.toLatin1();
        return py::str(s);
    }

    py::object qkeysequenceFromVoidPtr(const void* v) {
        const QKeySequence ks = *static_cast<const QKeySequence*>(v);
        return py::str(ks.toString().toStdString());
    }

    py::object qjsonarrayFromVoidPtr(const void* v) {
        const QJsonArray arr = *static_cast<const QJsonArray*>(v);
        py::list pyList;
        for (const auto& item : arr) {
            pyList.append(qvariantToPyObject(item.toVariant()));
        }
        return pyList;
    }

    py::object qjsonobjectFromVoidPtr(const void* v) {
        const QJsonObject obj = *static_cast<const QJsonObject*>(v);
        py::dict pyDict;
        for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
            pyDict[py::str(it.key().toStdString())] = qvariantToPyObject(it.value().toVariant());
        }
        return pyDict;
    }

    py::object qjsonvalueFromVoidPtr(const void* v) {
        const QJsonValue val = *static_cast<const QJsonValue*>(v);
        return qvariantToPyObject(val.toVariant());
    }

    py::object qjsondocumentFromVoidPtr(const void* v) {
        const QJsonDocument doc = *static_cast<const QJsonDocument*>(v);
        if (doc.isArray()) {
            const auto array = doc.array();
            return qjsonarrayFromVoidPtr(&array);
        }
        if (doc.isObject()) {
            const auto object = doc.object();
            return qjsonobjectFromVoidPtr(&object);
        }
        return py::none();
    }

    static std::unordered_map<QMetaType::Type, PyObjectFromVoidPtrFunc> specializedPodVoidPtrToPyObjectConverters = {
        {QMetaType::Int, podToPyObject<int>},
        {QMetaType::UInt, podToPyObject<unsigned int>},
        {QMetaType::LongLong, podToPyObject<qlonglong>},
        {QMetaType::ULongLong, podToPyObject<qulonglong>},
        {QMetaType::Long, podToPyObject<long>},
        {QMetaType::ULong, podToPyObject<unsigned long>},
        {QMetaType::Short, podToPyObject<short>},
        {QMetaType::UShort, podToPyObject<unsigned short>},
        {QMetaType::Float, podToPyObject<float>},
        {QMetaType::Double, podToPyObject<double>},
        {QMetaType::Bool, podToPyObject<bool>},
        {QMetaType::Char, podToPyObject<char>},
        {QMetaType::UChar, podToPyObject<uchar>},
        {QMetaType::QByteArray, memoryViewFromQByteaArrayAsVoidPtr},
        {QMetaType::QPoint, qpointFromVoidPtr},
        {QMetaType::QSize, qsizeFromVoidPtr},
        {QMetaType::QRect, qrectFromVoidPtr},
        {QMetaType::QColor, qcolorFromVoidPtr},
        {QMetaType::QDateTime, qdatetimeFromVoidPtr},
        {QMetaType::QDate, qdateFromVoidPtr},
        {QMetaType::QTime, qtimeFromVoidPtr},
        {QMetaType::QUuid, quuidFromVoidPtr},
        {QMetaType::QUrl, qurlFromVoidPtr},
        {QMetaType::QString, qstringFromVoidPtr},
        {QMetaType::QPointF, qpointfromVoidPtr},
        {QMetaType::QSizeF, qsizefFromVoidPtr},
        {QMetaType::QRectF, qrectfFromVoidPtr},
        {QMetaType::QVector2D, qvector2dFromVoidPtr},
        {QMetaType::QVector3D, qvector3dFromVoidPtr},
        {QMetaType::QVector4D, qvector4dFromVoidPtr},
        {QMetaType::QQuaternion, quaternionFromVoidPtr},
        {QMetaType::QMatrix4x4, matrix4x4FromVoidPtr},
        {QMetaType::QChar, qcharFromVoidPtr},
        {QMetaType::QKeySequence, qkeysequenceFromVoidPtr},
        {QMetaType::QJsonArray, qjsonarrayFromVoidPtr},
        {QMetaType::QJsonObject, qjsonobjectFromVoidPtr},
        {QMetaType::QJsonValue, qjsonvalueFromVoidPtr},
        {QMetaType::QJsonDocument, qjsondocumentFromVoidPtr}};

    py::object qmetatypeToPyObject(int typeId, const void* data) {
        if (specializedPodVoidPtrToPyObjectConverters.contains(static_cast<QMetaType::Type>(typeId))) {
            return specializedPodVoidPtrToPyObjectConverters.at(static_cast<QMetaType::Type>(typeId))(data);
        }
        return qvariantToPyObject(QVariant(static_cast<QMetaType>(typeId), data));
    }

    void addMetatypeVoidPtrToPyObjectConverterFunc(QMetaType::Type type, PyObjectFromVoidPtrFunc&& func) {
        specializedPodVoidPtrToPyObjectConverters.insert({type, std::move(func)});
    }
} // namespace qtpyt