#include <gtest/gtest.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

#include <QVariant>
#include <QString>
#include <QByteArray>
#include <QPointF>

#include "qtpyt/conversions.h"

namespace py = pybind11;

static std::unique_ptr<py::scoped_interpreter> s_py_guard;

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

int main(int argc, char** argv) {
    s_py_guard = std::make_unique<py::scoped_interpreter>(); // initialize Python once
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}