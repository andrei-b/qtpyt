#define  PYBIND11_NO_KEYWORDS
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include "qtpyt/qpyscript.h"
#include "qtpyt/q_py_shared_array.h"
#include <filesystem>
#include <QPointF>
#include <QSizeF>
#include <QSignalSpy>
#include <gtest/gtest.h>
#include <QObject>
#include <QDebug>
#include "testobject.h"



static std::filesystem::path testdata_path(std::string_view rel) {
    // If WORKING_DIRECTORY is set to the binary dir, this works:
    return std::filesystem::path("pyfiles") / rel;
}


class QPyScriptTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        qtpyt::registerSharedArray<float>("const QPySharedArray<float>&", true);
        qtpyt::registerSharedArray<double>("QPySharedArray<double>", true);
        qtpyt::registerSharedArray<long long>(" QPySharedArray<long long>", true);
        auto newid = qtpyt::registerContainerType<QList<int>>("QList<int>");
        qtpyt::registerContainerType<QList<QVector3D>>("QList<QVector3D>");
        qWarning() <<" Registered std::list<int> with type id " << newid;
    }
    static void TearDownTestSuite() {
        // runs once after all tests in this fixture
    }
};

TestObj obj;

TEST_F(QPyScriptTest, RunScriptFileGlobal) {
    QObject root_obj;
    auto [success, errorMsg] = qtpyt::QPyScript::runScriptFileGlobal(
        QString::fromStdString(testdata_path("module1.py").string()), &obj);
    EXPECT_TRUE(success) << "Error running script: " << errorMsg.toStdString();
}

TEST_F(QPyScriptTest, CallsTestObjMethodFromScript) {

    QSignalSpy spy(&obj, SIGNAL(methodCalled(QString)));
    ASSERT_TRUE(spy.isValid());

    auto [success, errorMsg] = qtpyt::QPyScript::runScriptFileGlobal(
        QString::fromStdString(testdata_path("module2.py").string()), &obj);
    ASSERT_TRUE(success) << errorMsg.toStdString();

    ASSERT_GE(spy.count(), 1);

    const auto args = spy.takeFirst();
    EXPECT_EQ(args.at(0).toString(), QStringLiteral("setPoints called"));
}

TEST_F(QPyScriptTest, CallsTestObjMethodReturningValueFromScript) {

    auto [success, errorMsg] = qtpyt::QPyScript::runScriptFileGlobal(
        QString::fromStdString(testdata_path("module3.py").string()), &obj);
    ASSERT_TRUE(success) << errorMsg.toStdString();

}
