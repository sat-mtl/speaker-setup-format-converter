#include "mainwindow.hpp"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    QApplication::setApplicationName("Spatial Format Converter");
    QApplication::setOrganizationName("Société des Arts Technologiques");
    QApplication::setOrganizationDomain("sat.qc.ca");

    MainWindow window;
    window.show();
    
    return app.exec();
}
