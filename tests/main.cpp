#include  <gtest/gtest.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <qtpyt/q_py_thread_pool.h>

namespace py = pybind11;

static std::unique_ptr<py::scoped_interpreter> s_py_guard;
static std::unique_ptr<py::gil_scoped_release> s_gil_release;

int main(int argc, char** argv) {
    qtpyt::QPyThreadPool::initialize(1, false);
    s_py_guard = std::make_unique<py::scoped_interpreter>();
    printf("Py_GetVersion: %s\n", Py_GetVersion());
    printf("Py_GetBuildInfo: %s\n", Py_GetBuildInfo());
    ::testing::InitGoogleTest(&argc, argv);
    const auto result = RUN_ALL_TESTS();
    qtpyt::QPyThreadPool::instance().shutdown();
    return result;
}