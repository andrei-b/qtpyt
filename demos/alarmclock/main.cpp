#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "alarmmodel.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName("QtAlarmClock");
    QGuiApplication::setOrganizationName("ExampleOrg");

    AlarmModel alarmModel;
    alarmModel.load();

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("alarmModel", &alarmModel);

    // Load the QML module by URI + root component name
    engine.loadFromModule("AlarmApp", "Main");

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
