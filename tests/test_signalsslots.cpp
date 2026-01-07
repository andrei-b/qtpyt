#include "../include/qtpyt/qpyslot.h"
#include "qtpyt/qpysharedarray.h"
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
    }
    static void TearDownTestSuite() {
        // runs once after all tests in this fixture
    }
};


TEST_F(QPyScriptTest, TestSlotCalledFromPython) {
  TestObj obj;
  auto m = qtpyt::QPyModule(QString::fromStdString(testdata_path("module6.py").string()),
                                    qtpyt::QPySourceType::File);
    m.addVariable<QObject*>("obj", &obj);
    qtpyt::QPySlot::connectPythonFunction(&obj, "passPoint(QPoint)", m, "slot", qtpyt::QPyRegisteredType(QMetaType::Void));
    obj.setIntProperty(69);
    obj.emitPassPoint(QPoint(12, 24));
    int p = obj.intProperty();
    EXPECT_EQ(p, 36);
}

TEST_F(QPyScriptTest, TestSlotCalledFromPython2) {
    TestObj obj;
    auto m = qtpyt::QPyModule(QString::fromStdString(testdata_path("module6.py").string()),
                                      qtpyt::QPySourceType::File);
    m.addVariable<QObject*>("obj", &obj);
    m.makeSlot("slot_2", QMetaType::Void).connectToSignal(&obj, &TestObj::passPoint);
    obj.setIntProperty(69);
    obj.emitPassPoint(QPoint(12, 24));
    auto value = obj.value();
    EXPECT_EQ(value, QPoint(12, 24));
}