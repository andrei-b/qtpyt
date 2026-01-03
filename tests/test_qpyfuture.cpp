#include <gtest/gtest.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

#include "qtpyt/qpymodule.h"

namespace py = pybind11;

TEST(QPyFuture, QPyFutureRun) {
    auto m = qtpyt::QPyModule("def test_func(x, y):\n"
                           "    return x + y\n", qtpyt::QPySourceType::SourceString);
    // create a Python-callable function from a C++ lambda that doubles its input
    QVariantList args = {2.5, 3.5};
    qtpyt::QPyFuture future(m, nullptr,"test_func", "Double", std::move(args));
    future.run();
    auto res = future.resultAsVariant(0);
    EXPECT_EQ(res, 6.0);
}

