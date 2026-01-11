#include <QApplication>
#include "mainwindow.h"
#include <qtpyt/globalinit.h>
#include <qtpyt/qpythreadpool.h>

#include "../../include/qtpyt/globalinit.h"
#include "../../include/qtpyt/qpythreadpool.h"

int main(int argc, char** argv) {
    qtpyt::init();
    qtpyt::QPyThreadPool::instance().initialize(1);

    QApplication app(argc, argv);

    // If you pass QObject*/QWidget* through QVariant anywhere else, registering can help.
    // (Here we pass the api object via addVariable, which uses QVariant internally.)
    qRegisterMetaType<QObject*>("QObject*");

    MainWindow w;
    w.show();

    return app.exec();
}
