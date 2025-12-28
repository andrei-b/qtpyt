#include <gtest/gtest.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

#include "qtpyt/qpymodule.h"

namespace py = pybind11;

static std::unique_ptr<py::scoped_interpreter> s_py_guard;

TEST(QPyModule, QPyFutureRun) {
    auto m = std::make_shared<qtpyt::QPyModule> ("def test_func(x, y):\n"
                           "    return x + y\n", qtpyt::QPySourceType::SourceString, "test_func");
    // create a Python-callable function from a C++ lambda that doubles its input
    QVariantList args = {2.5, 3.5};
    qtpyt::QPyFuture future(m, "test_func", std::move(args));
    future.run();
    auto res = future.resultAsVariant(QMetaType(QMetaType::Double), 0);
    EXPECT_EQ(res, 6.0);
}

