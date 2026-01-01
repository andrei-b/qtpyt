#define  PYBIND11_NO_KEYWORDS
#include <pybind11/pybind11.h>
#include "../include/qtpyt/qpyslot.h"
#include "qtpyt/qpysharedarray.h"
#include <qtpyt/qpyslot.h>
#include <filesystem>
#include <QSignalSpy>
#include <gtest/gtest.h>
#include <QObject>
#include  <QVector3D>
#include <QDebug>
#include "testobject.h"

class AsycSlotsTest: public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        qtpyt::registerSharedArray<float>("const QPySharedArray<float>&", true);
        auto newid = qtpyt::registerContainerType<QList<int>>("QList<int>");
        qtpyt::registerContainerType<std::map<int, QString>>("std::map<int, QString>");

    }
    static void TearDownTestSuite() {
        // runs once after all tests in this fixture
    }
};

TEST_F(AsycSlotsTest, CallAsyncSlot) {
    TestObj obj;
    auto m = qtpyt::QPyModule::create("import qembed\n"
                                      "def slot_async(p):\n"
                                      "    qembed.set_property(obj, 'intProperty', p[0] + p[1])\n"
                                      "    print(f'Async slot called with point: {p}, set intProperty to {p[0] + p[1]}')\n"
                                      "    return p[0] + p[1]", qtpyt::QPySourceType::SourceString);
    m->addVariable<QObject*>("obj", &obj);
    QSharedPointer<qtpyt::QPyFutureNotifier> notifier = QSharedPointer<qtpyt::QPyFutureNotifier>::create();
    int result = 0;
    QObject::connect(notifier.data(), &qtpyt::QPyFutureNotifier::finished, [&obj, &result](const QVariant& res) {
        result = res.toInt();
    });
    auto slot = m->makeSlot("slot_async", QMetaType::Int, notifier);
    slot.connectAsyncToSignal(&obj, &TestObj::passPoint);
    obj.setIntProperty(0);
    obj.emitPassPoint(QPoint(10, 20));
    // Wait for async slot to complete
    while (result == 0) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        QThread::msleep(1);
    }
    int p = obj.intProperty();
    EXPECT_EQ(p, 30);
}
