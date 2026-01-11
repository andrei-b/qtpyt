#include "qtpyt/qpyscript.h"
#include <filesystem>
#include <QPointF>
#include <QSizeF>
#include <QSignalSpy>
#include <gtest/gtest.h>
#include <QObject>
#include  <QVector3D>
#include <QDebug>
#include "testobject.h"



static std::filesystem::path testdata_path(std::string_view rel) {
    // If WORKING_DIRECTORY is set to the binary dir, this works:
    return std::filesystem::path("pyfiles") / rel;
}


class QPyScriptTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
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

TEST_F(QPyScriptTest, CallPassingAndReturningQList) {
    auto [success, errorMsg] = qtpyt::QPyScript::runScriptFileGlobal(
        QString::fromStdString(testdata_path("module4.py").string()), &obj);
    ASSERT_TRUE(success) << errorMsg.toStdString();
}

TEST_F(QPyScriptTest, SetReadPropertyFromPython) {
    auto [success, errorMsg] = qtpyt::QPyScript::runScriptFileGlobal(
        QString::fromStdString(testdata_path("module5.py").string()), &obj);
    ASSERT_TRUE(success) << errorMsg.toStdString();
    EXPECT_EQ(obj.property("intProperty").value<int>(), 111);
    EXPECT_EQ(obj.property("value").value<QPoint>(), QPoint(24, 56));
}