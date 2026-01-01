#include <gtest/gtest.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

#include <QVariant>
#include <QString>
#include <QByteArray>
#include <QPointF>
#include <QVector3D>

#include "qtpyt/conversions.h"
#include  <qtpyt/qpysharedarray.h>

namespace py = pybind11;

TEST(Conversions, IntRoundtrip) {
    QVariant in = 42;
    py::object obj = qtpyt::qvariantToPyObject(in);
    auto outOpt = qtpyt::pyObjectToQVariant(obj, QByteArray());
    ASSERT_TRUE(outOpt.has_value());
    EXPECT_EQ(outOpt->toInt(), in.toInt());
}

TEST(Conversions, StringRoundtrip) {
    QVariant in = QStringLiteral("hello from test");
    py::object obj = qtpyt::qvariantToPyObject(in);
    auto outOpt = qtpyt::pyObjectToQVariant(obj, QByteArray());
    ASSERT_TRUE(outOpt.has_value());
    EXPECT_EQ(outOpt->toString(), in.toString());
}


TEST(Conversions, QPointRoundtrip) {
    QPoint point(10, 20);
    QVariant in = QVariant::fromValue(point);
    py::object obj = qtpyt::qvariantToPyObject(in);
    auto outOpt = qtpyt::pyObjectToQVariant(obj, QByteArray("QPoint"));
    ASSERT_TRUE(outOpt.has_value());
    QPoint out = outOpt->value<QPoint>();
    EXPECT_EQ(out, point);
}

TEST(Conversions, QByteArrayRoundtrip) {
    QByteArray byteArray("test data", 9);
    QVariant in = QVariant::fromValue(byteArray);
    py::object obj = qtpyt::qvariantToPyObject(in);
    auto outOpt = qtpyt::pyObjectToQVariant(obj, QByteArray("QByteArray"));
    ASSERT_TRUE(outOpt.has_value());
    QByteArray out = outOpt->value<QByteArray>();
    EXPECT_EQ(out, byteArray);
}

TEST(Conversions, BytesToQByteArray) {
    py::bytes pyBytes("sample bytes data");
    auto outOpt = qtpyt::pyObjectToQVariant(pyBytes, QByteArray("QByteArray"));
    ASSERT_TRUE(outOpt.has_value());
    QByteArray out = outOpt->value<QByteArray>();
    EXPECT_EQ(out, QByteArray("sample bytes data"));
}

TEST(Convesions, QMapTrivialRoundtrip) {
    qtpyt::registerQMapType<int, QString>("QMap<int, QString>");
    QMap<int, QString> map = {{1, "Ananas"}, {2, "Banana"}, {3, "Citron"}};
    QVariant in = QVariant::fromValue(map);
    py::object obj = qtpyt::qvariantToPyObject(in);
    auto outOpt = qtpyt::pyObjectToQVariant(obj, QByteArray("QMap<int, QString>"));
    ASSERT_TRUE(outOpt.has_value());
    QMap<int, QString> out = outOpt->value<QMap<int, QString>>();
    EXPECT_EQ(out, map);
}

TEST(Conversions, QVector3DRoundTrip) {
    QVector3D vec(1.0f, 2.0f, 3.0f);
    QVariant in = QVariant::fromValue(vec);
    py::object obj = qtpyt::qvariantToPyObject(in);
    auto outOpt = qtpyt::pyObjectToQVariant(obj, QByteArray("QVector3D"));
    ASSERT_TRUE(outOpt.has_value());
    QVector3D out = outOpt->value<QVector3D>();
    EXPECT_EQ(out, vec);
}

TEST(Conversions, QVectorDoubleRoundTrip) {
    qtpyt::registerContainerType<QVector<double>>("QVector<double>");
    QVector<double> vec = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f};
    QVariant in = QVariant::fromValue(vec);
    py::object obj = qtpyt::qvariantToPyObject(in);
    qWarning() << "  " << py::repr(obj).cast<std::string>();
    auto outOpt = qtpyt::pyObjectToQVariant(obj, QByteArray("QVector<double>"));
    ASSERT_TRUE(outOpt.has_value());
    QVector<double> out = outOpt->value<QVector<double>>();
    ASSERT_EQ(out.size(), vec.size());
    for (int i = 0; i < vec.size(); ++i) {
        EXPECT_EQ(out[i], vec[i]);
    }
}

TEST(Conversions, QVectorQVector3DRoundTrip) {
    qtpyt::registerContainerType<QVector<QVector3D>>("QVector<QVector3D>");
    QVector<QVector3D> vec = {{1.0f, 2.0f, 3.0f}, {4.0f, 5.0f, 6.0f}};
    QVariant in = QVariant::fromValue(vec);
    py::object obj = qtpyt::qvariantToPyObject(in);
    qWarning() << "  " << py::repr(obj).cast<std::string>();
    auto outOpt = qtpyt::pyObjectToQVariant(obj, QByteArray("QVector<QVector3D>"));
    ASSERT_TRUE(outOpt.has_value());
    QVector<QVector3D> out = outOpt->value<QVector<QVector3D>>();
    ASSERT_EQ(out.size(), vec.size());
    for (int i = 0; i < vec.size(); ++i) {
        EXPECT_EQ(out[i], vec[i]);
    }
}

TEST(Conversions, QListQPointRoundTrip) {
    qtpyt::registerContainerType<QList<QPoint>>("QList<QPoint>");
    QList<QPoint> list = {{100, 200}, {300, 400}, {500, 600}};
    QVariant in = QVariant::fromValue(list);
    py::object obj = qtpyt::qvariantToPyObject(in);
    auto outOpt = qtpyt::pyObjectToQVariant(obj, QByteArray("QList<QPoint>"));
    ASSERT_TRUE(outOpt.has_value());
    QList<QPoint> out = outOpt->value<QList<QPoint>>();
    ASSERT_EQ(out.size(), list.size());
    for (int i = 0; i < list.size(); ++i) {
        EXPECT_EQ(out[i], list[i]);
    }
}

TEST(Conversions, QListQPStringRoundTrip) {
    qtpyt::registerContainerType<QList<QString>>("QList<QString>");
    QList<QString> list = {"first", "second", "third"};
    QVariant in = QVariant::fromValue(list);
    py::object obj = qtpyt::qvariantToPyObject(in);
    auto outOpt = qtpyt::pyObjectToQVariant(obj, QByteArray("QList<QString>"));
    ASSERT_TRUE(outOpt.has_value());
    auto out = outOpt->value<QList<QString>>();
    ASSERT_EQ(out.size(), list.size());
    for (int i = 0; i < list.size(); ++i) {
        EXPECT_EQ(out[i], list[i]);
    }
}

TEST(Conversions, QSharedArrayRoundTrip) {
    qtpyt::registerSharedArray<float>("QPySharedArray<float>");
    QVector<float> data = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    qtpyt::QPySharedArray<float> sharedArray(6);
    for (int i = 0; i < data.size(); ++i) {
        sharedArray[i] = data[i];
    }
    QVariant in = QVariant::fromValue(sharedArray);
    py::object obj = qtpyt::qvariantToPyObject(in);
    auto outOpt = qtpyt::pyObjectToQVariant(obj, QByteArray("QPySharedArray<float>"));
    ASSERT_TRUE(outOpt.has_value());
    auto out = outOpt->value<qtpyt::QPySharedArray<float>>();
    ASSERT_EQ(out.size(), sharedArray.size());
    for (int i = 0; i < sharedArray.size(); ++i) {
        EXPECT_EQ(out[i], sharedArray[i]);
    }
}