#include <gtest/gtest.h>
#include <qtpyt/qpymodulebase.h>

using namespace qtpyt;

// Test fixture for PyRunner tests
class PyRunnerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

// Test that PyRunner can be constructed
TEST_F(PyRunnerTest, Construction) {
    PyRunner runner;
    EXPECT_FALSE(runner.isInitialized());
}

// Test initialization
TEST_F(PyRunnerTest, Initialize) {
    PyRunner runner;
    EXPECT_TRUE(runner.initialize());
    EXPECT_TRUE(runner.isInitialized());
    EXPECT_TRUE(runner.getLastError().empty());
}

// Test double initialization fails
TEST_F(PyRunnerTest, DoubleInitializeFails) {
    PyRunner runner;
    EXPECT_TRUE(runner.initialize());
    EXPECT_FALSE(runner.initialize());
    EXPECT_FALSE(runner.getLastError().empty());
}

// Test executing script without initialization fails
TEST_F(PyRunnerTest, ExecuteScriptWithoutInitialize) {
    PyRunner runner;
    EXPECT_FALSE(runner.executeScript("print('Hello')"));
    EXPECT_EQ(runner.getLastError(), "PyRunner not initialized");
}

// Test executing empty script fails
TEST_F(PyRunnerTest, ExecuteEmptyScript) {
    PyRunner runner;
    runner.initialize();
    EXPECT_FALSE(runner.executeScript(""));
    EXPECT_EQ(runner.getLastError(), "Empty script provided");
}

// Test executing script after initialization succeeds
TEST_F(PyRunnerTest, ExecuteScriptAfterInitialize) {
    PyRunner runner;
    runner.initialize();
    EXPECT_TRUE(runner.executeScript("print('Hello, World!')"));
    EXPECT_TRUE(runner.getLastError().empty());
}

// Test setting Python path
TEST_F(PyRunnerTest, SetPythonPath) {
    PyRunner runner;
    std::vector<std::string> paths = {"/usr/lib/python3", "/opt/python"};
    runner.setPythonPath(paths);
    // No exception should be thrown
    SUCCEED();
}

// Test getLastError returns correct error message
TEST_F(PyRunnerTest, GetLastError) {
    PyRunner runner;
    EXPECT_TRUE(runner.getLastError().empty());
    
    runner.executeScript("test");
    EXPECT_FALSE(runner.getLastError().empty());
}
