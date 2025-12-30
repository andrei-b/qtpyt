#define  PYBIND11_NO_KEYWORDS
#include <pybind11/pybind11.h>
#include "../src/qpyslot.h"
#include "qtpyt/q_py_shared_array.h"
#include <filesystem>
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
        qtpyt::registerSharedArray<float>("const QPySharedArray<float>&", true);
        auto newid = qtpyt::registerContainerType<QList<int>>("QList<int>");
        qtpyt::registerContainerType<QList<QVector3D>>("QList<QVector3D>");
        qtpyt::registerContainerType<std::map<int, QString>>("std::map<int, QString>");
        qWarning() <<" Registered std::list<int> with type id " << newid;
    }
    static void TearDownTestSuite() {
        // runs once after all tests in this fixture
    }
};


TEST_F(QPyScriptTest, TestSlotCalledFromPython) {
  /*  QObject root_obj;
    auto [success, errorMsg] = qtpyt::QPyScript::runScriptFileGlobal(
        QString::fromStdString(testdata_path("module5.py").string()), &obj);
    EXPECT_TRUE(success) << "Error running script: " << errorMsg.toStdString();*/
}

