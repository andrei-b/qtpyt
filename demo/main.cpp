#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Set application information
    QCoreApplication::setApplicationName("qtpyt Demo");
    QCoreApplication::setApplicationVersion("0.0.1");
    QCoreApplication::setOrganizationName("qtpyt");
    
    // Create and show main window
    MainWindow window;
    window.show();
    
    return app.exec();
}
