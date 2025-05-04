#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Optional: Set Application Info for better integration
    QCoreApplication::setOrganizationName("YourCompany"); // Change if needed
    QCoreApplication::setApplicationName("EZ Paster");
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);

    MainWindow w;
    w.show();
    return a.exec();
} 
