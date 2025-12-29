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

