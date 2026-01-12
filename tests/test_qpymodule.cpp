#include <filesystem>
#include <gtest/gtest.h>
#include "qtpyt/qpymodule.h"

#include <QVector3D>
#include <utility>
#include <utility>

#include "qtpyt/qpysharedbytearray.h"
#include "qtpyt/qpysequencereference.h"
#include "qtpyt/qpysequencewrapper.h"


class QPyFutureNotifier : public QObject, public qtpyt::IQPyFutureNotifier {
    Q_OBJECT
  public:
    static QSharedPointer<QPyFutureNotifier> createNotifier() {
        return QSharedPointer<QPyFutureNotifier>(new QPyFutureNotifier());
    }
    QPyFutureNotifier() = default;
    ~QPyFutureNotifier() override = default;
    void notifyStarted(const QString& function) override {
        emit started(function);
    }
    void notifyFinished(const QString& function, const QVariant& value = QVariant()) override{
        emit finished(function, value);
    }
    void notifyResultAvailable(const QString& function,const QVariant& value) override {
        emit resultAvailable(function, value);
    }
    void notifyErrorOccurred(const QString& function, const QString& errorMessage) override {
        emit errorOccurred(function, errorMessage);
    }
    signals:
      void started(const QString& function);
    void finished(const QString& function, const QVariant& value = QVariant());
    void resultAvailable(const QString& function, const QVariant& value);
    void errorOccurred(const QString& function, const QString& errorMessage);
};

static std::filesystem::path testdata_path(std::string_view rel) {
    // If WORKING_DIRECTORY is set to the binary dir, this works:
    return std::filesystem::path("pyfiles") / rel;
}

TEST(QPyModule, CallAsyncRunsAndReturns) {
    auto m = qtpyt::QPyModule("def test_func(x, y):\n"
                           "    return x + y\n", qtpyt::QPySourceType::SourceString);
    auto f = m.callAsync<double, double>("test_func", "double", 2.5, 3.5).value();
    f.waitForFinished();
    auto res = f.resultAs<double>(0);
    EXPECT_EQ(res, 6.0);
}

TEST(QPyModule, SyncRunsAfterAsync) {
    auto m = qtpyt::QPyModule(QString::fromStdString(testdata_path("module8.py").string()),
        qtpyt::QPySourceType::File);
    auto f = m.callAsync<double, double>("func_1", "double", 2.5, 3.5).value();
    auto func_2 = m.makeFunction<int()>("func_2");
    const auto res2 = func_2();
    EXPECT_EQ(res2, 1);
    auto f2 = m.callAsync<>("func_2", QMetaType::Int ).value();
    f2.waitForFinished();
    auto res = f2.resultAs<int>(0);
    EXPECT_EQ(res, 1);

}

TEST(QPyModule, CallAsyncWithVariantListRunsAndReturns) {
    auto m = qtpyt::QPyModule ("def test_func(x, y):\n"
                           "    return x + y\n", qtpyt::QPySourceType::SourceString);
    QVariantList args = {2.5, 3.5};
    auto f = m.callAsync("test_func", QMetaType::Double, 2.5, 3.5).value();
    f.waitForFinished();
    auto res = f.resultAs<double>(0);
    EXPECT_EQ(res, 6.0);
}

TEST(QPyModule, CallAsyncReturnVoid) {
    auto m = qtpyt::QPyModule("def test_func(x, y):\n"
                            "    print(f'Result is: {x + y}')\n", qtpyt::QPySourceType::SourceString);
    QVariant x = 2.5;
    QVariant y = 3.5;
    QVariantList args = {x, y};
    auto f = m.callAsync( "test_func", QMetaType::Void, x, y).value();
    f.waitForFinished();
    EXPECT_EQ(f.state(), qtpyt::QPyFutureState::Finished);
    EXPECT_EQ(f.resultCount(), 0);
}

TEST(QPyModule, CallAsyncReturnVoid2) {
    auto m = qtpyt::QPyModule("def test_func(x, y):\n"
                            "    print(f'Result is: {x + y}')\n", qtpyt::QPySourceType::SourceString);
    QVariant x = 2.5;
    QVariant y = 3.5;
    QVariantList args = {x, y};
    auto f = m.callAsync("test_func", "void", x, y).value();
    f.waitForFinished();
    EXPECT_EQ(f.state(), qtpyt::QPyFutureState::Finished);
    EXPECT_EQ(f.resultCount(), 0);
}


TEST(QPyModule, CallAsyncInvalidFunctionRuntimeError) {
    auto m = qtpyt::QPyModule("def test_func(x, y):\n"
                           "    return x + y\n", qtpyt::QPySourceType::SourceString);
    auto f = m.callAsync<>( "test_func", "double");
    f->waitForFinished();
    EXPECT_EQ(f->state(), qtpyt::QPyFutureState::Error);
    EXPECT_EQ(f->errorMessage().toStdString(),  "QPyFuture: Python error: TypeError: test_func() missing 2 required positional arguments: 'x' and 'y'");
}

TEST(QPyModule, TestMakeAsyncFunction) {
    auto m = qtpyt::QPyModule("def add_vectors(a, b):\n"
                           "    return (a[0]+b[0], a[1] + b[1], a[2] + b[2])\n", qtpyt::QPySourceType::SourceString);
    auto add_vectors = m.makeAsyncFunction<QVector3D, QVector3D, QVector3D>("add_vectors");
    auto f = add_vectors({1.0, 2.0, 3.0}, {5.0, 8.0, 10.0});
    f->waitForFinished();
    EXPECT_EQ(f.value().state(), qtpyt::QPyFutureState::Finished);
    EXPECT_EQ(f.value().resultAs<QVector3D>(0), QVector3D(6.0, 10.0, 13.0));
}


TEST(QPyModule, TestQPyFutureNotifier) {
    auto m = qtpyt::QPyModule("import time\n"
                           "def long_task():\n"
                           "    time.sleep(2)\n"
                           "    return 42\n", qtpyt::QPySourceType::SourceString);
    auto notifier = QPyFutureNotifier::createNotifier();
    int oresult;
    bool notified = false;
    QObject::connect(notifier.data(), &QPyFutureNotifier::finished, [&notified, &oresult](const QString& function, const QVariant& result) {
        EXPECT_EQ(function.toStdString(), "long_task");
        notified = true;
        oresult = result.toInt();
    });
    m.setFutureNotifier(notifier);
    auto f = m.callAsync<>( "long_task", QMetaType::Int);

    f->waitForFinished();
    EXPECT_EQ(f->state(), qtpyt::QPyFutureState::Finished);
    EXPECT_EQ(oresult, 42);
    EXPECT_EQ(f->resultAs<int>(0), 42);
    EXPECT_TRUE(notified);
}

TEST(QPyModule, TestAddFunction) {
    auto m = qtpyt::QPyModule(
                "def call_test_func():\n"
                         "    test_func()\n",
                          qtpyt::QPySourceType::SourceString);
    bool called = false;
    std::function<void()> test_func = [&called]() {
        called = true;
    };
    m.addFunction<void>("test_func", std::move(test_func));
    m.call<void>("call_test_func");
    EXPECT_TRUE(called);
}

TEST(QPyModule, TestAddFunctionWithReturn) {
    auto m = qtpyt::QPyModule(
                "def call_test_func():\n"
                         "    return test_func()\n",
                          qtpyt::QPySourceType::SourceString);
    std::function<int()> test_func = []() {
        return 1234;
    };
    m.addFunction<int>("test_func", std::move(test_func));
    int res = m.call<int>("call_test_func");
    EXPECT_EQ(res, 1234);
}

TEST (QPyModule, TestAddFunctionWithArgs) {
    auto m = qtpyt::QPyModule(
                "def call_test_func(x, y):\n"
                         "    return test_func(x, y)\n",
                          qtpyt::QPySourceType::SourceString);
    std::function<double(double, double)> test_func = [](double a, double b) {
        return a * b;
    };
    m.addFunction<double, double, double>("test_func", std::move(test_func));
    double res = m.call<double>("call_test_func", 3.0, 4.0);
    EXPECT_EQ(res, 12.0);
}

TEST(QPyModule, TestAsyncFunctionWithVectorWrapper) {
    bool finish =false;
    bool error = false;
    std::function<void()> n_finish = [&finish]() {
        finish = true;
    };
    std::function<void()> n_error = [&error]() {
        error = true;
    };

    class AsyncNotifier : public QPyFutureNotifier {
    public:
        AsyncNotifier(std::function<void()> onFinish,
                      std::function<void()> onError)
            : onFinish_(std::move(onFinish)), onError_(std::move(onError)) {}
        void notifyStarted(const QString& f)  override {
        }
        void notifyFinished(const QString& f, const QVariant& value = QVariant()) override{
            onFinish_();
        }
        void notifyResultAvailable(const QString& f, const QVariant& value) override {
        }
        void notifyErrorOccurred(const QString& f, const QString& errorMessage) override {
            onError_();
        }
    private:
        std::function<void()> onFinish_;
        std::function<void()> onError_;
    };

    auto m = qtpyt::QPyModule(QString::fromStdString(testdata_path("module8.py").string()),
                           qtpyt::QPySourceType::File);
    m.setFutureNotifier(QSharedPointer<AsyncNotifier>(new AsyncNotifier(n_finish, n_error)));
    auto scale_array = m.makeAsyncFunction<void, qtpyt::QPySequenceReference, double>(
        "scale_array");
    {
        auto arr = QSharedPointer<QVector<double>>::create(4096);
        for (int i = 0; i < arr->length(); ++i) {
            (*arr)[i] = static_cast<double>(i);
        }
        auto wrapped = qtpyt::wrapVectorWithSequenceReference(arr, false);
        scale_array(wrapped, 2.5);
    }
    int count = 0;
    while (!finish && !error) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        QThread::msleep(1);
        count++;
        if (count > 120600) {
            qWarning() << "TestAsyncFunctionWithVectorWrapper: timeout waiting for finish\nIt may be not a bug, just your CPU being too slow";
            break;
        }
    }
    EXPECT_TRUE(finish);
    EXPECT_FALSE(error);
}

TEST(QPyModule, TestAsyncFunctionWithVectorWrapperShares) {
    bool finish =false;
    bool error = false;
    std::function<void()> n_finish = [&finish]() {
        finish = true;
    };
    std::function<void()> n_error = [&error]() {
        error = true;
    };

    class AsyncNotifier : public QPyFutureNotifier {
    public:
        AsyncNotifier(std::function<void()> onFinish,
                      std::function<void()> onError)
            : onFinish_(std::move(onFinish)), onError_(std::move(onError)) {}
        void notifyStarted(const QString& f)  override {
        }
        void notifyFinished(const QString& f, const QVariant& value = QVariant()) override{
            onFinish_();
        }
        void notifyResultAvailable(const QString& f, const QVariant& value) override {
        }
        void notifyErrorOccurred(const QString& f, const QString& errorMessage) override {
            onError_();
        }
    private:
        std::function<void()> onFinish_;
        std::function<void()> onError_;
    };

    auto m = qtpyt::QPyModule(QString::fromStdString(testdata_path("module8.py").string()),
                           qtpyt::QPySourceType::File);
    m.setFutureNotifier(QSharedPointer<AsyncNotifier>(new AsyncNotifier(n_finish, n_error)));
    auto scale_array = m.makeAsyncFunction<void, qtpyt::QPySequenceReference, double>("scale_array");

        auto arr = QSharedPointer<QVector<float>>::create(4096);
        auto wrapped = qtpyt::wrapVectorWithSequenceReference(arr, false);
        for (int i = 0; i < arr->length(); ++i) {
            (*arr)[i] = static_cast<float>(i);
        }
        scale_array(wrapped, 2.5);
   // (*arr)[0] = 100000.0f; // to check that the array is really shared=

    int count = 0;
    while (!finish && !error) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        QThread::msleep(1);
        count++;
        if (count > 120600) {
            qWarning() << "TestAsyncFunctionWithVectorWrapperShares: timeout waiting for finish\nIt may be not a bug, just your CPU being too slow";
            break;
        }
    }
    for (int i = 0; i < arr->length(); ++i) {
        EXPECT_DOUBLE_EQ((*arr)[i], static_cast<double>(i) * 2.5);
    }
    EXPECT_TRUE(finish);
    EXPECT_FALSE(error);
}

TEST(QPyModule, TestAsyncFunctionWithVectorWrapperReadOnly) {
    auto m = qtpyt::QPyModule(QString::fromStdString(testdata_path("module8.py").string()),
                           qtpyt::QPySourceType::File);
    auto scale_array = m.makeAsyncFunction<void, qtpyt::QPySequenceReference, int>("scale_array");
        auto arr = QSharedPointer<QVector<long long>>::create(4096);
        auto wrapped = qtpyt::wrapVectorWithSequenceReference(arr, true);
        for (int i = 0; i < arr->length(); ++i) {
            (*arr)[i] = static_cast<float>(i);
        }
        auto f = scale_array(wrapped, 25);
        f->waitForFinished();
    EXPECT_EQ(f->state(), qtpyt::QPyFutureState::Error);
    EXPECT_TRUE(f->errorMessage().startsWith("QPyFutureImpl::run: Python error: TypeError: cannot modify read-only memory"));
}

TEST(QPyModule, TestAsyncFunctionWithVectorWrapperReadOnly2) {
    auto m = qtpyt::QPyModule(QString::fromStdString(testdata_path("module8.py").string()),
                           qtpyt::QPySourceType::File);
    auto scale_array = m.makeAsyncFunction<int, qtpyt::QPySequenceReference>("summ_array");
    auto arr = QSharedPointer<QVector<long long>>::create(4096);
    auto wrapped = qtpyt::wrapVectorWithSequenceReference(arr, true);
    for (int i = 0; i < arr->length(); ++i) {
        (*arr)[i] = static_cast<float>(i);
    }
    auto f = scale_array(wrapped);
    f->waitForFinished();
    EXPECT_EQ(f->state(), qtpyt::QPyFutureState::Finished);
    auto res = f->resultAs<int>(0);
    EXPECT_EQ(res, 4095 * 4096 / 2);
}

TEST(QPyModule, TestAsyncFunctionWithVectorWrapperReturned) {
    auto m = qtpyt::QPyModule(QString::fromStdString(testdata_path("module8.py").string()),
                           qtpyt::QPySourceType::File);
    auto make_array = m.makeAsyncFunction<qtpyt::QPySequenceReference, int>("make_array");

    auto f =  make_array(1024);
    f->waitForFinished();
    EXPECT_EQ(f->state(), qtpyt::QPyFutureState::Finished);
    auto seq = f->resultAs<qtpyt::QPySequenceReference>(0);
    auto arr = qtpyt::QPySequenceWrapper<int>(seq);
    ASSERT_EQ(arr.size(), 1024);
    for (int i = 0; i < arr.size(); ++i) {
        EXPECT_EQ(arr[i], i);
    }
}

TEST(QPyModule, TestPassingVectorWrapperReturned) {
   auto m = qtpyt::QPyModule(QString::fromStdString(testdata_path("module8.py").string()),
                           qtpyt::QPySourceType::File);
    auto make_array = m.makeAsyncFunction<qtpyt::QPySequenceReference, short int>("make_array");

    auto scale_array = m.makeAsyncFunction<void, qtpyt::QPySequenceReference, int>("scale_array");
    auto f =  make_array(1024);
    f->waitForFinished();
    EXPECT_EQ(f->state(), qtpyt::QPyFutureState::Finished);
    auto seq = f->resultAs<qtpyt::QPySequenceReference>(0);
    f = scale_array(seq, 3);
    f->waitForFinished();
    EXPECT_EQ(f->state(), qtpyt::QPyFutureState::Finished);
    auto arr = qtpyt::QPySequenceWrapper<int>(seq);
    ASSERT_EQ(arr.size(), 1024);
    for (int i = 0; i < arr.size(); ++i) {
        EXPECT_EQ(arr[i], i*3);
    }
}

TEST(QPyModule, TestQPySharedByteArrayShared) {
    auto m = qtpyt::QPyModule("def test(b):\n"
                                                   "    for i in range(len(b)):\n"
                                                   "        b[i] = (i * 2) % 256\n"
                                                   "    return None\n",
                                                   qtpyt::QPySourceType::SourceString);
    auto test = m.makeAsyncFunction<void, qtpyt::QPySharedByteArray>("test");

    qtpyt::QPySharedByteArray sba;
    sba.resize(64);
    for (int i = 0; i < sba.size(); ++i) {
        sba[i] = 0;
    }
    auto f =  test(sba);
    f->waitForFinished();
    EXPECT_EQ(f->state(), qtpyt::QPyFutureState::Finished);
    for (int i = 0; i < sba.size(); ++i) {
        EXPECT_EQ(sba[i], i*2 % 256);
    }
}

TEST(QPyModule, TestQPySharedByteArrayReadOnly) {
    auto m = qtpyt::QPyModule("def test(b):\n"
                                                   "    print(b)\n"
                                                   "    print(type(b))\n"
                                                   "    print(len(b))\n"
                                                   "    for i in range(len(b)):\n"
                                                   "        b[i] = (i * 2) % 256\n"
                                                   "    return None\n",
                                                   qtpyt::QPySourceType::SourceString);
    auto test = m.makeAsyncFunction<void, qtpyt::QPySharedByteArray>("test");

    qtpyt::QPySharedByteArray sba;
    sba.resize(64);
    for (int i = 0; i < sba.size(); ++i) {
        sba[i] = 'a';
    }
    sba.setReadOnly(true);
    auto f =  test(sba);
    f->waitForFinished();
    EXPECT_EQ(f->state(), qtpyt::QPyFutureState::Error);
    EXPECT_TRUE(f->errorMessage().startsWith("QPyFutureImpl::run: Python error: TypeError: cannot modify read-only memory"));
    for (int i = 0; i < sba.size(); ++i) {
        EXPECT_EQ(sba[i], 'a');
    }
}

TEST(QPyModule, TestQByteArrayReadOnly2) {
    auto m = qtpyt::QPyModule(QString::fromStdString(testdata_path("module8.py").string()),
            qtpyt::QPySourceType::File);
    auto test = m.makeAsyncFunction<int, qtpyt::QPySharedByteArray>("summ_array");

    qtpyt::QPySharedByteArray sba;
    sba.resize(64);
    for (int i = 0; i < sba.size(); ++i) {
        sba[i] = 'a';
    }
    sba.setReadOnly(true);
    auto f =  test(sba);
    f->waitForFinished();
    EXPECT_EQ(f->state(), qtpyt::QPyFutureState::Finished);
    auto res = f->resultAs<int>(0);
    EXPECT_EQ(res, 'a'*sba.size());
}


#include "test_qpymodule.moc"