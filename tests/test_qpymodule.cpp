#include <gtest/gtest.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

#include "qtpyt/qpymodule.h"

namespace py = pybind11;

TEST(QPyModule, CallAsyncRunsAndReturns) {
    auto m = qtpyt::QPyModule::create("def test_func(x, y):\n"
                           "    return x + y\n", qtpyt::QPySourceType::SourceString);
    auto f = m->callAsync<double, double>(nullptr, "test_func", "Double", 2.5, 3.5).value();
    f.waitForFinished();
    auto res = f.resultAs<double>(0);
    EXPECT_EQ(res, 6.0);
}

TEST(QPyModule, SyncRunsAfterAsync) {
    auto m = qtpyt::QPyModule::create(
              "import time\n"
        "def func_1(x, y):\n"
              "    time.sleep(1)\n"
              "    return x + y\n"
              "\n"
              "\n"
              "def func_2():"
              "    return 1", qtpyt::QPySourceType::SourceString);
    auto f = m->callAsync<double, double>(nullptr,"func_1", "Double", 2.5, 3.5).value();
    auto func_2 = m->makeFunction<int()>("func_2");
    const auto res2 = func_2();
    EXPECT_EQ(res2, 1);
    auto f2 = m->callAsync<>(nullptr, "func_2", QMetaType::Int ).value();
    f2.waitForFinished();
    auto res = f2.resultAs<int>(0);
    EXPECT_EQ(res, 1);

}

TEST(QPyModule, CallAsyncWithVariantListRunsAndReturns) {
    auto m = qtpyt::QPyModule::create ("def test_func(x, y):\n"
                           "    return x + y\n", qtpyt::QPySourceType::SourceString);
    QVariantList args = {2.5, 3.5};
    auto f = m->callAsync(nullptr, "test_func", QMetaType::Double, std::move(args)).value();
    f.waitForFinished();
    auto res = f.resultAs<double>(0);
    EXPECT_EQ(res, 6.0);
}

TEST(QPyModule, CallAsyncReturnVoid) {
    auto m = qtpyt::QPyModule::create("def test_func(x, y):\n"
                            "    print(f'Result is: {x + y}')\n", qtpyt::QPySourceType::SourceString);
    QVariant x = 2.5;
    QVariant y = 3.5;
    QVariantList args = {x, y};
    auto f = m->callAsync(nullptr, "test_func", QMetaType::Void, std::move(args)).value();
    f.waitForFinished();
    EXPECT_EQ(f.state(), qtpyt::QPyFutureState::Finished);
    EXPECT_EQ(f.resultCount(), 0);
}

TEST(QPyModule, CallAsyncReturnVoid2) {
    auto m = qtpyt::QPyModule::create("def test_func(x, y):\n"
                            "    print(f'Result is: {x + y}')\n", qtpyt::QPySourceType::SourceString);
    QVariant x = 2.5;
    QVariant y = 3.5;
    QVariantList args = {x, y};
    auto f = m->callAsync(nullptr, "test_func", "void", std::move(args)).value();
    f.waitForFinished();
    EXPECT_EQ(f.state(), qtpyt::QPyFutureState::Finished);
    EXPECT_EQ(f.resultCount(), 0);
}


TEST(QPyModule, CallAsyncInvalidFunctionRuntimeError) {
    auto m = qtpyt::QPyModule::create("def test_func(x, y):\n"
                           "    return x + y\n", qtpyt::QPySourceType::SourceString);
    auto f = m->callAsync<>(nullptr, "test_func", "Double");
    f->waitForFinished();
    EXPECT_EQ(f->state(), qtpyt::QPyFutureState::Error);
    EXPECT_EQ(f->errorMessage(), "Python error: TypeError: test_func() missing 2 required positional arguments: 'x' and 'y'");
}