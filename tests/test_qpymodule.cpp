#include <gtest/gtest.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

#include "qtpyt/qpymodule.h"

namespace py = pybind11;

TEST(QPyModule, CallAsyncRunsAndReturns) {
    auto m = std::make_shared<qtpyt::QPyModule> ("def test_func(x, y):\n"
                           "    return x + y\n", qtpyt::QPySourceType::SourceString, "test_func");
    auto f = m->callAsync<double, double>("test_func", 2.5, 3.5).value();
    f.waitForFinished();
    auto res = f.resultAt<double>(0);
    EXPECT_EQ(res, 6.0);
}

TEST(QPyModule, SyncRunsAfterAsync) {
    auto m = std::make_shared<qtpyt::QPyModule> (
              "import time\n"
        "def func_1(x, y):\n"
              "    time.sleep(1)\n"
              "    return x + y\n"
              "\n"
              "\n"
              "def func_2():"
              "    return 1", qtpyt::QPySourceType::SourceString, "func_1");
    auto f = m->callAsync<double, double>("func_1", 2.5, 3.5).value();
    auto func_2 = m->makeFunction<int()>("func_2");
    const auto res2 = func_2();
    EXPECT_EQ(res2, 1);
    auto f2 = m->callAsync<>("func_2" ).value();
    f2.waitForFinished();
    auto res = f2.resultAt<int>(0);
    EXPECT_EQ(res, 1);

}

TEST(QPyModule, CallAsyncWithVariantListRunsAndReturns) {
    auto m = std::make_shared<qtpyt::QPyModule> ("def test_func(x, y):\n"
                           "    return x + y\n", qtpyt::QPySourceType::SourceString, "test_func");
    QVariantList args = {2.5, 3.5};
    auto f = m->callAsync("test_func", std::move(args)).value();
    f.waitForFinished();
    auto res = f.resultAt<double>(0);
    EXPECT_EQ(res, 6.0);
}

TEST(QPyModule, CallAsyncInvalidFunctionRuntimeError) {
    auto m = std::make_shared<qtpyt::QPyModule> ("def test_func(x, y):\n"
                           "    return x + y\n", qtpyt::QPySourceType::SourceString, "test_func");
    auto f = m->callAsync<>("test_func");
    f->waitForFinished();
    EXPECT_EQ(f->state(), qtpyt::QPyFutureState::Error);
    EXPECT_EQ(f->errorMessage(), "Python error: TypeError: test_func() missing 2 required positional arguments: 'x' and 'y'");
}