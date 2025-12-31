#define  PYBIND11_NO_KEYWORDS
#include <pybind11/pybind11.h>
#include "../include/qtpyt/qpyslot.h"
#include "qtpyt/q_py_shared_array.h"
#include <qtpyt/qpyslot.h>
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
  TestObj obj;
  auto m = qtpyt::QPyModule::create(QString::fromStdString(testdata_path("module6.py").string()),
                                    qtpyt::QPySourceType::File);
    m->addVariable<QObject*>("obj", &obj);
    qtpyt::QPySlot::connectPythonFunction(&obj, "passPoint(QPoint)", m, "slot", qtpyt::QPyRegisteredType(QMetaType::Void));
    obj.setIntProperty(69);
    obj.emitPassPoint(QPoint(12, 24));
    int p = obj.intProperty();
    EXPECT_EQ(p, 36);
}

