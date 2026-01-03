#include <gtest/gtest.h>
#include <qtpyt/qpythreadpool.h>
#include <qtpyt/globalinit.h>

#include <iostream>

int main(int argc, char** argv) {
    qtpyt::init(true);
    qtpyt::QPyThreadPool::initialize(1, false);
    ::testing::InitGoogleTest(&argc, argv);
    const int result = RUN_ALL_TESTS();

    qtpyt::QPyThreadPool::instance().shutdown();
    return result;
}
