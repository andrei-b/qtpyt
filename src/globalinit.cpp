#include "../include/qtpyt/globalinit.h"

#include "pybind11/embed.h"
#include <pybind11/complex.h>
#include "conversions.h"
#include "internal/sharedarrayinternal.h"
#include <qlogging.h>
#include <QDebug>
#include <QString>
#include <QList>
#include <QPoint>
#include <QPointF>
#include <QVector3D>
#include <QVector4D>
#include <QQuaternion>
#include <QMatrix4x4>
#include <complex>
#include "pymodule.h"

namespace qtpyt {
    bool g_PyInitialized = false;

    void init(bool printPyInfo) {
        if (!g_PyInitialized) {
            pybind11::initialize_interpreter();
            g_PyInitialized = true;
            if (printPyInfo) {
                pybind11::gil_scoped_acquire gil;
                pybind11::module_ sys = pybind11::module_::import("sys");
                pybind11::object version = sys.attr("version");
                pybind11::object platform = sys.attr("platform");
                qInfo() << "Python initialized.";
                qInfo() << "Version:" << QString::fromStdString(version.cast<std::string>());
                qInfo() << "Platform:" << QString::fromStdString(platform.cast<std::string>());
            }
                makeEmbeddedModule();
                registerSharedArray<int>("QPySharedArray<int>");
                registerSharedArray<double>("QPySharedArray<double>");
                registerSharedArray<float>("QPySharedArray<float>");
                registerSharedArray<unsigned long>("QPySharedArray<unsigned long>");
                registerSharedArray<long>("QPySharedArray<long>");
                registerSharedArray<char>("QPySharedArray<char>");
                registerSharedArray<unsigned char>("QPySharedArray<unsigned char>");
                registerSharedArray<short>("QPySharedArray<short>");
                registerSharedArray<unsigned short>("QPySharedArray<unsigned short>");
                registerSharedArray<unsigned int>("QPySharedArray<unsigned int>");
                registerSharedArray<long long>("QPySharedArray<long long>");
                registerSharedArray<unsigned long long>("QPySharedArray<unsigned long long>");
                registerSharedArray<bool>("QPySharedArray<bool>");
                registerContainerType<QList<double>>("QList<double>");
                registerContainerType<QList<int>>("QList<int>");
                registerContainerType<QList<QString>>("QList<QString>");
                registerContainerType<QList<QVariant>>("QList<QVariant>");
                registerContainerType<QList<float>>("QList<float>");
                registerContainerType<QList<unsigned int>>("QList<unsigned int>");
                registerContainerType<QList<long long>>("QList<long long>");
                registerContainerType<QList<unsigned long long>>("QList<unsigned long long>");
                registerContainerType<QList<QPoint>>("QList<QPoint>");
                registerContainerType<QList<QPointF>>("QList<QPointF>");
                registerContainerType<QList<QVector3D>>("QList<QVector3D>");
                registerContainerType<QList<QVector4D>>("QList<QVector4D>");

        }
    }

} // namespace qtpyt