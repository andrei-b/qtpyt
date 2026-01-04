#include "../include/qtpyt/globalinit.h"

#include "pybind11/embed.h"
#include "internal/sharedarrayinternal.h"
#include <qlogging.h>
#include <QDebug>
#include <QString>

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
            }

        }
    }

} // namespace qtpyt