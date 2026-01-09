#include <filesystem>
#include <gtest/gtest.h>
#include "qtpyt/qpymodule.h"
#include <QPoint>

TEST(QPyModuleBase, MakeFunctionRunsAndReturns) {
    qtpyt::QPyModuleBase m("def test_func(x, y):\n"
                           "    return x + y\n", qtpyt::QPySourceType::SourceString);
    // create a Python-callable function from a C++ lambda that doubles its input
    const auto test_func = m.makeFunction<double(double, double)>("test_func");
    const auto res = test_func(2.5, 3.5);
    EXPECT_EQ(res, 6.0);
}

TEST(QPyModuleBase, MakeFunctionRunsAndReturnsFloat) {
    qtpyt::QPyModuleBase m("def test_func(x, y):\n"
                           "    return x + y\n", qtpyt::QPySourceType::SourceString);
    const auto test_func = m.makeFunction<float(float, float)>("test_func");
    const auto res = test_func(2.5f, 3.5f);
    EXPECT_FLOAT_EQ(res, 6.0f);
}

TEST(QPyModuleBase, MakeFunctionRunsAndReturnsInt) {
    qtpyt::QPyModuleBase m("def test_func(x, y):\n"
                           "    return x + y\n", qtpyt::QPySourceType::SourceString);
    const auto test_func = m.makeFunction<int(int, int)>("test_func");
    const auto res = test_func(2, 3);
    EXPECT_EQ(res, 5);
}

TEST(QPyModuleBase, MakeFunctionRunsAndReturnsLongLong) {
    qtpyt::QPyModuleBase m("def test_func(x, y):\n"
                           "    return x + y\n", qtpyt::QPySourceType::SourceString);
    const auto test_func = m.makeFunction<long long(long long, long long)>("test_func");
    const auto res = test_func(2LL, 3LL);
    EXPECT_EQ(res, 5LL);
}

TEST(QPyModuleBase, MakeQVarinatListAsParamters) {
    qtpyt::QPyModuleBase m("def test_func(x, y):\n"
                               "    return x + y\n", qtpyt::QPySourceType::SourceString);
    QVariantList args = {QString("Hello, "), QString("world!")};
    const auto res = m.call("test_func", QMetaType::QString, args);
    ASSERT_TRUE(res.first.has_value());
    EXPECT_EQ(res.first, "Hello, world!");
}

TEST(QPyModuleBase, TestQPointReturnValue) {
    qtpyt::QPyModuleBase m("def test_func(x, y):\n"
                               "    return (x, y)\n", qtpyt::QPySourceType::SourceString);
    QVariantList args = {10, 20};
    const auto res = m.call("test_func", QMetaType::QPoint, args);
    ASSERT_TRUE(res.first.has_value());
    EXPECT_EQ(res.first, QPoint(10,20));
}