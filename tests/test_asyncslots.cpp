#include "qtpyt/qpysharedarray.h"
#include <qtpyt/qpyslot.h>
#include <filesystem>
#include <QSignalSpy>
#include <gtest/gtest.h>
#include <QObject>
#include  <QVector3D>
#include <QDebug>
#include "testobject.h"
#include <qtpyt/qpyfuture.h>
#include "TestObject_2.h"

class AsycSlotsTest1: public ::testing::Test {
protected:
    static QCoreApplication* app;

    static void SetUpTestSuite() {
        if (!QCoreApplication::instance()) {
            static int argc = 1;
            static char* argv[] = {(char*)"test"};
            app = new QCoreApplication(argc, argv);
        }
    }

    static void TearDownTestSuite() {
        delete app;
        app = nullptr;
    }
};

QCoreApplication* AsycSlotsTest1::app = nullptr;

class QPyFutureNotifier1 : public QObject, public qtpyt::IQPyFutureNotifier {
    Q_OBJECT
  public:
    static QSharedPointer<QPyFutureNotifier1> createNotifier() {
        return QSharedPointer<QPyFutureNotifier1>(new QPyFutureNotifier1());
    }
    QPyFutureNotifier1() = default;
    ~QPyFutureNotifier1() override = default;
    void notifyStarted() {
        emit started();
    }
    void notifyFinished(const QVariant& value = QVariant()) override{
        emit finished(value);
    }
    void notifyResultAvailable(const QVariant& value) override {
        emit resultAvailable(value);
    }
    void notifyErrorOccurred(const QString& errorMessage) override {
        emit errorOccurred(errorMessage);
    }
    signals:
      void started();
    void finished(const QVariant& value = QVariant());
    void resultAvailable(const QVariant& value);
    void errorOccurred(const QString& errorMessage);
};


class AsycSlotsTest: public ::testing::Test {
protected:
    static void SetUpTestSuite() {

    }
    static void TearDownTestSuite() {
        // runs once after all tests in this fixture
    }
};

TEST_F(AsycSlotsTest, CallAsyncSlot) {
    TestObj obj;
    auto m = qtpyt::QPyModule("import qt_interop\n"
                                      "def slot_async(p):\n"
                                      "    print(f'Async slot called with point: {p}, set intProperty to {p[0] + p[1]}')\n"
                                      "    qt_interop.set_property(obj, 'intProperty', p[0] + p[1])\n"
                                      "    return p[0] + p[1]", qtpyt::QPySourceType::SourceString);
    m.addVariable<QObject*>("obj", &obj);
    QSharedPointer<QPyFutureNotifier1> notifier = QSharedPointer<QPyFutureNotifier1>::create();
    int result = 0;
    QObject::connect(notifier.data(), &QPyFutureNotifier1::finished, [&obj, &result](const QVariant& res) {
        result = res.toInt();
    });
    auto slot = m.makeSlot("slot_async", QMetaType::Int, notifier);
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

TEST_F(AsycSlotsTest, CallAsyncSlotIntArg) {
    TestObject_2 object;
    auto m = qtpyt::QPyModule("import qt_interop\n"
                                      "def slot_int(i):\n"
                                      "    if i == 42:"
                                      "    qt_interop.set_property(obj, 'success', True)\n",
                                        qtpyt::QPySourceType::SourceString);
    m.addVariable<QObject*>("obj", &object);
    QSharedPointer<QPyFutureNotifier1> notifier = QSharedPointer<QPyFutureNotifier1>::create();
    bool called = false;
    QObject::connect(notifier.data(), &QPyFutureNotifier1::finished, [&called](const QVariant& res) {
        called = true;
    });
    auto slot = m.makeSlot("slot_int", QMetaType::Void, notifier);
    slot.connectAsyncToSignal(&object, &TestObject_2::passInt);
    object.setSuccessProperty(false);
    object.emitInt(42);
    // Wait for async slot to complete
    int count = 0;
    while (!called) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        QThread::msleep(1);
        if (count++ > 200) {
            break;
        }
    }
    EXPECT_EQ(called, true);
    EXPECT_EQ(object.success(), true);
}

TEST_F(AsycSlotsTest, CallAsyncSlotStringArg) {
    TestObject_2 object;
    auto m = qtpyt::QPyModule("import qt_interop\n"
                                      "def slot_string(s):\n"
                                      "    if s == 'hello':\n"
                                      "        qt_interop.set_property(obj, 'success', True)\n",
                                        qtpyt::QPySourceType::SourceString);
    m.addVariable<QObject*>("obj", &object);
    QSharedPointer<QPyFutureNotifier1> notifier = QSharedPointer<QPyFutureNotifier1>::create();
    bool called = false;
    QObject::connect(notifier.data(), &QPyFutureNotifier1::finished, [&called](const QVariant& res) {
        called = true;
    });
    auto slot = m.makeSlot("slot_string", QMetaType::Void, notifier);
    slot.connectAsyncToSignal(&object, &TestObject_2::passString);
    object.setSuccessProperty(false);
    object.emitString("hello");
    // Wait for async slot to complete
    int count = 0;
    while (!called) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        QThread::msleep(1);
        if (count++ > 200) {
            break;
        }
    }
    EXPECT_EQ(called, true);
    EXPECT_EQ(object.success(), true);
}

TEST_F(AsycSlotsTest, CallAsyncSlotPointArg) {
    TestObject_2 object;
    auto m = qtpyt::QPyModule("import qt_interop\n"
                                      "def slot_point(p):\n"
                                      "    if p[0] == 5 and p[1] == 10:\n"
                                      "        qt_interop.set_property(obj, 'success', True)\n",
                                        qtpyt::QPySourceType::SourceString);
    m.addVariable<QObject*>("obj", &object);
    QSharedPointer<QPyFutureNotifier1> notifier = QSharedPointer<QPyFutureNotifier1>::create();
    bool called = false;
    QObject::connect(notifier.data(), &QPyFutureNotifier1::finished, [&called](const QVariant& res) {
        called = true;
    });
    auto slot = m.makeSlot("slot_point", QMetaType::Void, notifier);
    slot.connectAsyncToSignal(&object, &TestObject_2::passPoint);
    object.setSuccessProperty(false);
    object.emitPoint(QPoint(5, 10));
    // Wait for async slot to complete
    int count = 0;
    while (!called) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        QThread::msleep(1);
        if (count++ > 200) {
            break;
        }
    }
    EXPECT_EQ(called, true);
    EXPECT_EQ(object.success(), true);
}

TEST_F(AsycSlotsTest, CallAsyncSlotVector3DArg) {
    TestObject_2 object;
    auto m = qtpyt::QPyModule("import qt_interop\n"
                                      "def slot_vec3(v):\n"
                                      "    if v[0] == 1.0 and v[1] == 2.0 and v[2] == 3.0:\n"
                                      "        qt_interop.set_property(obj, 'success', True)\n",
                                        qtpyt::QPySourceType::SourceString);
    m.addVariable<QObject*>("obj", &object);
    QSharedPointer<QPyFutureNotifier1> notifier = QSharedPointer<QPyFutureNotifier1>::create();
    bool called = false;
    QObject::connect(notifier.data(), &QPyFutureNotifier1::finished, [&called](const QVariant& res) {
        called = true;
    });
    auto slot = m.makeSlot("slot_vec3", QMetaType::Void, notifier);
    slot.connectAsyncToSignal(&object, &TestObject_2::passVector3D);
    object.setSuccessProperty(false);
    object.emitVector3D(QVector3D(1.0f, 2.0f, 3.0f));
    // Wait for async slot to complete
    int count = 0;
    while (!called) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        QThread::msleep(1);
        if (count++ > 200) {
            break;
        }
    }
    EXPECT_EQ(called, true);
    EXPECT_EQ(object.success(), true);
}

TEST_F(AsycSlotsTest, CallAsyncSlotVariantArg) {
    TestObject_2 object;
    auto m = qtpyt::QPyModule("import qt_interop\n"
                                      "def slot_variant(v):\n"
                                      "    print(\"v=\", v)\n"
                                      "    if v == 3.14:\n"
                                      "        qt_interop.set_property(obj, 'success', True)\n",
                                        qtpyt::QPySourceType::SourceString);
    m.addVariable<QObject*>("obj", &object);
    QSharedPointer<QPyFutureNotifier1> notifier = QSharedPointer<QPyFutureNotifier1>::create();
    bool called = false;
    QObject::connect(notifier.data(), &QPyFutureNotifier1::finished, [&called](const QVariant& res) {
        called = true;
    });
    auto slot = m.makeSlot("slot_variant", QMetaType::Void, notifier);
    slot.connectAsyncToSignal(&object, &TestObject_2::passVariant);
    object.setSuccessProperty(false);
    object.emitVariant(QVariant(3.14));
    // Wait for async slot to complete
    int count = 0;
    while (!called) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        QThread::msleep(1);
        if (count++ > 200) {
            break;
        }
    }
    EXPECT_EQ(called, true);
    EXPECT_EQ(object.success(), true);
}

TEST_F(AsycSlotsTest, CallAsyncSlotVariantListArg) {
    TestObject_2 object;
    auto m = qtpyt::QPyModule("import qt_interop\n"
                                      "def slot_variant_list(v):\n"
                                      "    if v == (1, 2, 3, 4, 5):\n"
                                      "        qt_interop.set_property(obj, 'success', True)\n",
                                        qtpyt::QPySourceType::SourceString);
    m.addVariable<QObject*>("obj", &object);
    QSharedPointer<QPyFutureNotifier1> notifier = QSharedPointer<QPyFutureNotifier1>::create();
    bool called = false;
    QObject::connect(notifier.data(), &QPyFutureNotifier1::finished, [&called](const QVariant& res) {
        called = true;
    });
    auto slot = m.makeSlot("slot_variant_list", QMetaType::Void, notifier);
    slot.connectAsyncToSignal(&object, &TestObject_2::passVariantList);
    object.setSuccessProperty(false);
    object.emitVariantList(QVariantList{1, 2, 3, 4, 5});
    // Wait for async slot to complete
    int count = 0;
    while (!called) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        QThread::msleep(1);
        if (count++ > 200) {
            break;
        }
    }
    EXPECT_EQ(object.success(), true);
}

TEST_F(AsycSlotsTest, CallAsyncSlotIntListArg) {
    TestObject_2 object;
    auto m = qtpyt::QPyModule("import qt_interop\n"
                                      "def slot_int_list(v):\n"
                                      "    if v == (10, 20, 30):\n"
                                      "        qt_interop.set_property(obj, 'success', True)\n",
                                        qtpyt::QPySourceType::SourceString);
    m.addVariable<QObject*>("obj", &object);
    QSharedPointer<QPyFutureNotifier1> notifier = QSharedPointer<QPyFutureNotifier1>::create();
    bool called = false;
    QObject::connect(notifier.data(), &QPyFutureNotifier1::finished, [&called](const QVariant& res) {
        called = true;
    });
    auto slot = m.makeSlot("slot_int_list", QMetaType::Void, notifier);
    slot.connectAsyncToSignal(&object, &TestObject_2::passIntList);
    object.setSuccessProperty(false);
    object.emitIntList(QList<int>{10, 20, 30});
    // Wait for async slot to complete
    int count = 0;
    while (!called) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        QThread::msleep(1);
        if (count++ > 200) {
            break;
        }
    }
    EXPECT_EQ(object.success(), true);
}

TEST_F(AsycSlotsTest, CallAsyncSlotStringIntMapArg) {
    TestObject_2 object;
    auto m = qtpyt::QPyModule("import qt_interop\n"
                                      "def slot_string_int_map(m):\n"
                                      "    print('m=', m)\n"
                                      "    if m == {'a': 1, 'b': 2}:\n"
                                      "        qt_interop.set_property(obj, 'success', True)\n",
                                        qtpyt::QPySourceType::SourceString);
    m.addVariable<QObject*>("obj", &object);
    QSharedPointer<QPyFutureNotifier1> notifier = QSharedPointer<QPyFutureNotifier1>::create();
    bool called = false;
    QObject::connect(notifier.data(), &QPyFutureNotifier1::finished, [&called](const QVariant& res) {
        called = true;
    });
    auto slot = m.makeSlot("slot_string_int_map", QMetaType::Void, notifier);
    slot.connectAsyncToSignal(&object, &TestObject_2::passStringIntMap);
    object.setSuccessProperty(false);
    object.emitStringIntMap(QMap<QString, int>{{"a", 1}, {"b", 2}});
    // Wait for async slot to complete
    int count = 0;
    while (!called) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        QThread::msleep(1);
        if (count++ > 200) {
            break;
        }
    }
    EXPECT_EQ(object.success(), true);
}

TEST_F(AsycSlotsTest, CallAsyncSlotStringAndIntArg) {
    TestObject_2 object;
    auto m = qtpyt::QPyModule("import qt_interop\n"
                                      "def slot_string_and_int(s, i):\n"
                                      "    if s == 'value' and i == 99:\n"
                                      "        qt_interop.set_property(obj, 'success', True)\n",
                                        qtpyt::QPySourceType::SourceString);
    m.addVariable<QObject*>("obj", &object);
    QSharedPointer<QPyFutureNotifier1> notifier = QSharedPointer<QPyFutureNotifier1>::create();
    bool called = false;
    QObject::connect(notifier.data(), &QPyFutureNotifier1::finished, [&called](const QVariant& res) {
        called = true;
    });
    auto slot = m.makeSlot("slot_string_and_int", QMetaType::Void, notifier);
    slot.connectAsyncToSignal(&object, &TestObject_2::passStringAndInt);
    object.setSuccessProperty(false);
    object.emitStringAndInt("value", 99);
    // Wait for async slot to complete
    int count = 0;
    while (!called) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        QThread::msleep(1);
        if (count++ > 200) {
            break;
        }
    }
    EXPECT_EQ(object.success(), true);
}

TEST_F(AsycSlotsTest, CallAsyncSlotSharedArrayArg) {
    TestObject_2 object;
    auto m = qtpyt::QPyModule("import qt_interop\n"
                                      "def slot_shared_array(arr):\n"
                                      "    if len(arr) == 3 and arr[0] == 1.1 and arr[1] == 2.2 and arr[2] == 3.3:\n"
                                      "        qt_interop.set_property(obj, 'success', True)\n",
                                        qtpyt::QPySourceType::SourceString);
    m.addVariable<QObject*>("obj", &object);
    QSharedPointer<QPyFutureNotifier1> notifier = QSharedPointer<QPyFutureNotifier1>::create();
    bool called = false;
    QObject::connect(notifier.data(), &QPyFutureNotifier1::finished, [&called](const QVariant& res) {
        called = true;
    });
    auto slot = m.makeSlot("slot_shared_array", QMetaType::Void, notifier);
    slot.connectAsyncToSignal(&object, &TestObject_2::passSharedArray);
    object.setSuccessProperty(false);
    qtpyt::QPySharedArray<double> arr(3);
    arr[0] = 1.1;
    arr[1] = 2.2;
    arr[2] = 3.3;
    object.emitSharedArray(arr);
    // Wait for async slot to complete
    int count = 0;
    while (!called) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        QThread::msleep(1);
        if (count++ > 200) {
            break;
        }
    }
    EXPECT_EQ(object.success(), true);
}

TEST_F(AsycSlotsTest, CallAsyncSlotQPairArg) {
    TestObject_2 object;
    auto m = qtpyt::QPyModule("import qt_interop\n"
                                      "def slot_qpair(p):\n"
                                      "    if p[0] == 'key' and p[1] == 123:\n"
                                      "        qt_interop.set_property(obj, 'success', True)\n",
                                        qtpyt::QPySourceType::SourceString);
    m.addVariable<QObject*>("obj", &object);
    QSharedPointer<QPyFutureNotifier1> notifier = QSharedPointer<QPyFutureNotifier1>::create();
    bool called = false;
    QObject::connect(notifier.data(), &QPyFutureNotifier1::finished, [&called](const QVariant& res) {
        called = true;
    });
    auto slot = m.makeSlot("slot_qpair", QMetaType::Void, notifier);
    slot.connectAsyncToSignal(&object, &TestObject_2::passQPair);
    object.setSuccessProperty(false);
    QPair<QString, int> p("key", 123);
    object.emitQPair(p);
    // Wait for async slot to complete
    int count = 0;
    while (!called) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        QThread::msleep(1);
        if (count++ > 200) {
            break;
        }
    }
    EXPECT_EQ(object.success(), true);
}

TEST_F(AsycSlotsTest, CallAsyncSlotQVariantMapArg) {
    TestObject_2 object;
    auto m = qtpyt::QPyModule("import qt_interop\n"
                                      "def slot_qvariant_map(m):\n"
                                      "    if m == {'x': 10, 'y': 20}:\n"
                                      "        qt_interop.set_property(obj, 'success', True)\n",
                                        qtpyt::QPySourceType::SourceString);
    m.addVariable<QObject*>("obj", &object);
    QSharedPointer<QPyFutureNotifier1> notifier = QSharedPointer<QPyFutureNotifier1>::create();
    bool called = false;
    QObject::connect(notifier.data(), &QPyFutureNotifier1::finished, [&called](const QVariant& res) {
        called = true;
    });
    auto slot = m.makeSlot("slot_qvariant_map", QMetaType::Void, notifier);
    slot.connectAsyncToSignal(&object, &TestObject_2::passQVariantMap);
    object.setSuccessProperty(false);
    QVariantMap map;
    map.insert("x", 10);
    map.insert("y", 20);
    object.emitQVariantMap(map);
    // Wait for async slot to complete
    int count = 0;
    while (!called) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        QThread::msleep(1);
        if (count++ > 200) {
            break;
        }
    }
    EXPECT_EQ(object.success(), true);
}

TEST_F(AsycSlotsTest1, CallAsyncSlotWithAsync) {
    TestObj obj;
    auto m = qtpyt::QPyModule("import qt_interop\n"
                                      "def slot_async(p):\n"
                                      "    print(f'Async slot called with point: {p}, set intProperty to {p[0] + p[1]}')\n"
                                      "    qt_interop.set_property_async(obj, 'intProperty', p[0] + p[1])\n"
                                      "    return p[0] + p[1]", qtpyt::QPySourceType::SourceString);
    m.addVariable<QObject*>("obj", &obj);
    QSharedPointer<QPyFutureNotifier1> notifier = QSharedPointer<QPyFutureNotifier1>::create();
    int result = 0;
    QObject::connect(notifier.data(), &QPyFutureNotifier1::finished, [&obj, &result](const QVariant& res) {
        result = res.toInt();
    });
    auto slot = m.makeSlot("slot_async", QMetaType::Int, notifier);
    slot.connectAsyncToSignal(&obj, &TestObj::passPoint);
    obj.setIntProperty(0);
    obj.emitPassPoint(QPoint(10, 20));
    // Wait for async slot to complete
    while (result == 0) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        QThread::msleep(1);
    }
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    int p = obj.intProperty();
    EXPECT_EQ(p, 30);
}

TEST_F(AsycSlotsTest1, InvokeMTRoutine) {
    TestObj obj;
    auto m = qtpyt::QPyModule("import qt_interop\n"
                                      "def invoke_async():\n"
                                      "    d = qt_interop.invoke_mt(obj, 'retMap')\n"
                                      "    if d[1] == 'Apple':\n"
                                      "        return 1\n"
                                      "    return 0",
                                      qtpyt::QPySourceType::SourceString);
    m.addVariable<QObject*>("obj", &obj);
    QSharedPointer<QPyFutureNotifier1> notifier = QSharedPointer<QPyFutureNotifier1>::create();
    int result = 0;
    QObject::connect(notifier.data(), &QPyFutureNotifier1::finished, [&obj, &result](const QVariant& res) {
        result = res.toInt();
    });
    m.callAsync<>(notifier, "invoke_async", QMetaType::Int);
    int count = 0;
    while (result == 0) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        QThread::msleep(1);
        if (count++ > 200) {
            break;
        }
    }
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    EXPECT_EQ(result, 1);
}

TEST_F(AsycSlotsTest1, InvokeAsyncRoutine) {
    TestObj obj;
    auto m = qtpyt::QPyModule("import qt_interop\n"
                                      "def invoke_async():\n"
                                      "    qt_interop.invoke_async(obj, 'setSize', (10, 20))\n"
                                      "    return 1\n",
                                      qtpyt::QPySourceType::SourceString);
    m.addVariable<QObject*>("obj", &obj);
    QSharedPointer<QPyFutureNotifier1> notifier = QSharedPointer<QPyFutureNotifier1>::create();
    int result = 0;
    QObject::connect(notifier.data(), &QPyFutureNotifier1::finished, [&obj, &result](const QVariant& res) {
        result = res.toInt();
    });
    m.callAsync<>(notifier, "invoke_async", QMetaType::Int);
    int count = 0;
    while (result == 0) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        QThread::msleep(1);
        if (count++ > 200) {
            break;
        }
    }
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    EXPECT_EQ(result, 1);
    QString str = obj.getLastCalled();
    EXPECT_EQ(str, "qsize");
    QSize s = obj.sizeByQSize;
    EXPECT_EQ(s, QSize(10, 20));

}


#include "test_asyncslots.moc"