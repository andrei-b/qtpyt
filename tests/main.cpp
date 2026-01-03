#include <pybind11/embed.h>

#include <gtest/gtest.h>

#include <qtpyt/qpythreadpool.h>


#include <iostream>

int main(int argc, char** argv) {
    qtpyt::QPyThreadPool::initialize(1, false);

    pybind11::scoped_interpreter guard{};
    try {
        const auto sys = pybind11::module_::import("sys");
        const auto ver = pybind11::str(sys.attr("version")).cast<std::string>();
        std::cout << "Python version: " << ver << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to query Python version: " << e.what() << std::endl;
    }

    ::testing::InitGoogleTest(&argc, argv);
    const int result = RUN_ALL_TESTS();

    qtpyt::QPyThreadPool::instance().shutdown();
    return result;
}
