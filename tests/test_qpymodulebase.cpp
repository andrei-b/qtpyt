#include <filesystem>
#include <gtest/gtest.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

#include "qtpyt/qpymodule.h"
#include <qtpyt/qpysharedarray.h>
#include <QPoint>

namespace py = pybind11;

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
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(res, "Hello, world!");
}

TEST(QPyModuleBase, TestQPointReturnValue) {
    qtpyt::QPyModuleBase m("def test_func(x, y):\n"
                               "    return (x, y)\n", qtpyt::QPySourceType::SourceString);
    QVariantList args = {10, 20};
    const auto res = m.call("test_func", QMetaType::QPoint, args);
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(res, QPoint(10,20));
}

TEST(QPyModuleBase, TestQPySharedArrayShared) {
    qtpyt::registerSharedArray<double>("QPySharedArray<double>", true);
    qtpyt::QPySharedArray<double> arr(2);
    arr[0] = 1.23;
    arr[1] = 4.56;
    qtpyt::QPyModuleBase m("def test_func(arr):\n"
                               "    print(f'Before: {arr[1]}')\n"
                               "    arr[0] = 2.72\n"
                               "    arr[1] = 3.14\n"
                               "    print(f'After: {arr[1]}')\n", qtpyt::QPySourceType::SourceString);
    auto b = arr;
    m.call<void, qtpyt::QPySharedArray<double>>("test_func", std::move(b));
    EXPECT_DOUBLE_EQ(arr[0], 2.72);
    EXPECT_DOUBLE_EQ(arr[1], 3.14);
}

static std::filesystem::path testdata_path(std::string_view rel) {
    return std::filesystem::path("pyfiles") / rel;
}

TEST(QPyModuleBase, TestQPySharedArrayReturned) {
    qtpyt::registerSharedArray<int>("QPySharedArray<int>", true);
    qtpyt::QPyModuleBase m(QString::fromStdString(testdata_path("module7.py").string()), qtpyt::QPySourceType::File);
    qtpyt::QPySharedArray<int> a = m.call<qtpyt::QPySharedArray<int>>("make_memoryview" );
    EXPECT_EQ(a[0], 10);
    EXPECT_EQ(a[1], 20);
    EXPECT_EQ(a[2], 30);
    EXPECT_EQ(a[3], 40);
}

TEST(QPyModuleBase, TestQPySharedArrayReturned2) {
    qtpyt::registerSharedArray<double>("QPySharedArray<Double>", true);
    qtpyt::QPyModuleBase m(QString::fromStdString(testdata_path("module7.py").string()), qtpyt::QPySourceType::File);
    qtpyt::QPySharedArray<double> a = m.call<qtpyt::QPySharedArray<double>>("make_memoryview_2" );
    EXPECT_EQ(a[0], 1.0);
    EXPECT_EQ(a[1], 2.0);
    EXPECT_EQ(a[2], 3.0);
    EXPECT_EQ(a[3], 4.0);
}

TEST(QPyModuleBase, TestQPySharedArrayReturned3) {
    qtpyt::registerSharedArray<int>("QPySharedArray<int>", true);
    qtpyt::QPyModuleBase m(QString::fromStdString(testdata_path("module7.py").string()), qtpyt::QPySourceType::File);
    auto a = m.call("make_memoryview", "QPySharedArray<int>", {});
    auto b = a.value().value<qtpyt::QPySharedArray<int>>();
    EXPECT_EQ(b[0], 10);
    EXPECT_EQ(b[1], 20);
    EXPECT_EQ(b[2], 30);
    EXPECT_EQ(b[3], 40);
}

TEST(QPyModuleBase, TestQPySharedArrayReturned4) {
    qtpyt::registerSharedArray<double>("QPySharedArray<Double>", true);
    qtpyt::QPyModuleBase m(QString::fromStdString(testdata_path("module7.py").string()), qtpyt::QPySourceType::File);
    auto a = m.call("make_memoryview_2", "QPySharedArray<double>", {});
    auto b = a.value().value<qtpyt::QPySharedArray<double>>();
    EXPECT_EQ(b[0], 1.0);
    EXPECT_EQ(b[1], 2.0);
    EXPECT_EQ(b[2], 3.0);
    EXPECT_EQ(b[3], 4.0);
}