#include <QApplication>
#include "mainwindow.h"
#include <qtpyt/globalinit.h>
#include <qtpyt/qpythreadpool.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    qtpyt::init();
    qtpyt::QPyThreadPool::initialize(1, false);
    MainWindow w;
    w.show();
    return app.exec();
}
