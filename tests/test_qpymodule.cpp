#include <gtest/gtest.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

#include "qtpyt/qpymodule.h"

#include <QVector3D>

#include "qtpyt/qpysharedarray.h"

namespace py = pybind11;

TEST(QPyModule, CallAsyncRunsAndReturns) {
    auto m = qtpyt::QPyModule("def test_func(x, y):\n"
                           "    return x + y\n", qtpyt::QPySourceType::SourceString);
    auto f = m.callAsync<double, double>(nullptr, "test_func", "double", 2.5, 3.5).value();
    f.waitForFinished();
    auto res = f.resultAs<double>(0);
    EXPECT_EQ(res, 6.0);
}

TEST(QPyModule, SyncRunsAfterAsync) {
    auto m = qtpyt::QPyModule(
              "import time\n"
        "def func_1(x, y):\n"
              "    time.sleep(1)\n"
              "    return x + y\n"
              "\n"
              "\n"
              "def func_2():"
              "    return 1", qtpyt::QPySourceType::SourceString);
    auto f = m.callAsync<double, double>(nullptr,"func_1", "double", 2.5, 3.5).value();
    auto func_2 = m.makeFunction<int()>("func_2");
    const auto res2 = func_2();
    EXPECT_EQ(res2, 1);
    auto f2 = m.callAsync<>(nullptr, "func_2", QMetaType::Int ).value();
    f2.waitForFinished();
    auto res = f2.resultAs<int>(0);
    EXPECT_EQ(res, 1);

}

TEST(QPyModule, CallAsyncWithVariantListRunsAndReturns) {
    auto m = qtpyt::QPyModule ("def test_func(x, y):\n"
                           "    return x + y\n", qtpyt::QPySourceType::SourceString);
    QVariantList args = {2.5, 3.5};
    auto f = m.callAsync(nullptr, "test_func", QMetaType::Double, std::move(args)).value();
    f.waitForFinished();
    auto res = f.resultAs<double>(0);
    EXPECT_EQ(res, 6.0);
}

TEST(QPyModule, CallAsyncReturnVoid) {
    auto m = qtpyt::QPyModule("def test_func(x, y):\n"
                            "    print(f'Result is: {x + y}')\n", qtpyt::QPySourceType::SourceString);
    QVariant x = 2.5;
    QVariant y = 3.5;
    QVariantList args = {x, y};
    auto f = m.callAsync(nullptr, "test_func", QMetaType::Void, std::move(args)).value();
    f.waitForFinished();
    EXPECT_EQ(f.state(), qtpyt::QPyFutureState::Finished);
    EXPECT_EQ(f.resultCount(), 0);
}

TEST(QPyModule, CallAsyncReturnVoid2) {
    auto m = qtpyt::QPyModule("def test_func(x, y):\n"
                            "    print(f'Result is: {x + y}')\n", qtpyt::QPySourceType::SourceString);
    QVariant x = 2.5;
    QVariant y = 3.5;
    QVariantList args = {x, y};
    auto f = m.callAsync(nullptr, "test_func", "void", std::move(args)).value();
    f.waitForFinished();
    EXPECT_EQ(f.state(), qtpyt::QPyFutureState::Finished);
    EXPECT_EQ(f.resultCount(), 0);
}


TEST(QPyModule, CallAsyncInvalidFunctionRuntimeError) {
    auto m = qtpyt::QPyModule("def test_func(x, y):\n"
                           "    return x + y\n", qtpyt::QPySourceType::SourceString);
    auto f = m.callAsync<>(nullptr, "test_func", "double");
    f->waitForFinished();
    EXPECT_EQ(f->state(), qtpyt::QPyFutureState::Error);
    EXPECT_EQ(f->errorMessage(), "QPyFutureImpl::run: Python error: TypeError: test_func() missing 2 required positional arguments: 'x' and 'y'");
}

TEST(QPyModule, TestMakeAsyncFunction) {
    auto m = qtpyt::QPyModule("def add_vectors(a, b):\n"
                           "    return (a[0]+b[0], a[1] + b[1], a[2] + b[2])\n", qtpyt::QPySourceType::SourceString);
    auto add_vectors = m.makeAsyncFunction<QVector3D, QVector3D, QVector3D>(nullptr,"add_vectors");
    auto f = add_vectors({1.0, 2.0, 3.0}, {5.0, 8.0, 10.0});
    f->waitForFinished();
    EXPECT_EQ(f.value().state(), qtpyt::QPyFutureState::Finished);
    EXPECT_EQ(f.value().resultAs<QVector3D>(0), QVector3D(6.0, 10.0, 13.0));
}

TEST(QPyModule, TestAsincFunctionWithQSharedArray) {
    qtpyt::registerSharedArray<double>("QPySharedArray<double>", true);
    auto m = qtpyt::QPyModule("def scale_array(arr, factor):\n"
                           "    for i in range(len(arr)):\n"
                           "        arr[i] = arr[i] * factor\n", qtpyt::QPySourceType::SourceString);
    auto scale_array = m.makeAsyncFunction<void, qtpyt::QPySharedArray<double>, double>(nullptr,"scale_array");
    qtpyt::QPySharedArray<double> arr(3);
    arr[0] = 1.0;
    arr[1] = 2.0;
    arr[2] = 3.0;
    auto f = scale_array(arr, 2.5);
    f->waitForFinished();
    EXPECT_EQ(f.value().state(), qtpyt::QPyFutureState::Finished);
    EXPECT_DOUBLE_EQ(arr[0], 2.5);
    EXPECT_DOUBLE_EQ(arr[1], 5.0);
    EXPECT_DOUBLE_EQ(arr[2], 7.5);
}

TEST(QPyModule, TestQPyFutureNotifier) {
    auto m = qtpyt::QPyModule("import time\n"
                           "def long_task():\n"
                           "    time.sleep(2)\n"
                           "    return 42\n", qtpyt::QPySourceType::SourceString);
    auto notifier = QSharedPointer<qtpyt::QPyFutureNotifier>();
    int oresult;

    auto f = m.callAsync<>(notifier, "long_task", QMetaType::Int);
    bool notified = false;
    QObject::connect(notifier.data(), &qtpyt::QPyFutureNotifier::finished, [&notified, &oresult](const QVariant& result) {
        notified = true;
        oresult = result.toInt();
    });
    f->waitForFinished();
    EXPECT_EQ(f->state(), qtpyt::QPyFutureState::Finished);
    EXPECT_EQ(oresult, 42);
    EXPECT_EQ(f->resultAs<int>(0), 42);
    EXPECT_TRUE(notified);
}

TEST(QPyModule, TestAsyncReturningPySharedArray) {
    qtpyt::registerSharedArray<int>("QPySharedArray<int>", true);
    auto m = qtpyt::QPyModule("def create_array(n):\n"
                           "    arr = [i * 10 for i in range(n)]\n"
                           "    return arr\n", qtpyt::QPySourceType::SourceString);
    auto create_array = m.makeAsyncFunction<qtpyt::QPySharedArray<int>, int>(nullptr,"create_array");
    auto f = create_array(5);
    f->waitForFinished();
    EXPECT_EQ(f.value().state(), qtpyt::QPyFutureState::Finished);
    auto arr = f.value().resultAs<qtpyt::QPySharedArray<int>>(0);
    ASSERT_EQ(arr.size(), 5);
    EXPECT_EQ(arr[0], 0);
    EXPECT_EQ(arr[1], 10);
    EXPECT_EQ(arr[2], 20);
    EXPECT_EQ(arr[3], 30);
    EXPECT_EQ(arr[4], 40);
}

TEST(QPyModule, TestAddFunction) {
    auto m = qtpyt::QPyModule(
                "def call_test_func():\n"
                         "    test_func()\n",
                          qtpyt::QPySourceType::SourceString);
    bool called = false;
    std::function<void()> test_func = [&called]() {
        called = true;
    };
    m.addFunction<void>("test_func", std::move(test_func));
    m.call<void>("call_test_func");
    EXPECT_TRUE(called);
}

TEST(QPyModule, TestAddFunctionWithReturn) {
    auto m = qtpyt::QPyModule(
                "def call_test_func():\n"
                         "    return test_func()\n",
                          qtpyt::QPySourceType::SourceString);
    std::function<int()> test_func = []() {
        return 1234;
    };
    m.addFunction<int>("test_func", std::move(test_func));
    int res = m.call<int>("call_test_func");
    EXPECT_EQ(res, 1234);
}

TEST (QPyModule, TestAddFunctionWithArgs) {
    auto m = qtpyt::QPyModule(
                "def call_test_func(x, y):\n"
                         "    return test_func(x, y)\n",
                          qtpyt::QPySourceType::SourceString);
    std::function<double(double, double)> test_func = [](double a, double b) {
        return a * b;
    };
    m.addFunction<double, double, double>("test_func", std::move(test_func));
    double res = m.call<double>("call_test_func", 3.0, 4.0);
    EXPECT_EQ(res, 12.0);
}