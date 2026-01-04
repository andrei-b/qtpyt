#include "../src/conversions.h"
#include "registertypes.h"
#include <QList>
#include <QVector3D>

void registerTypes() {
    qtpyt::registerContainerType<QList<QVector3D>>("QList<QVector3D>");
    qtpyt::registerQPairType<QString, int>("QPair<QString, int>");
    qtpyt::registerContainerType<QList<int>>("QList<int>");
    qtpyt::registerContainerType<QList<QVector3D>>("QList<QVector3D>");
    qtpyt::registerContainerType<std::map<int, QString>>("std::map<int, QString>");
    qtpyt::registerContainerType<QList<int>>("QList<int>");
    qtpyt::registerContainerType<QList<QVector3D>>("QList<QVector3D>");
    qtpyt::registerContainerType<std::map<int, QString>>("std::map<int, QString>");
    qtpyt::registerContainerType<QList<int>>("QList<int>");
    qtpyt::registerContainerType<std::map<int, QString>>("std::map<int, QString>");
    qtpyt::registerQMapType<QString, int>("QMap<QString, int>");
    qtpyt::registerQPairType<QString, int>("QPair<QString, int>");


}