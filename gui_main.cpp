#include "mainwindow.hpp"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    QApplication::setApplicationName("Spatial Format Converter");
    QApplication::setOrganizationName("SAT-MTL");
    
    MainWindow window;
    window.show();
    
    return app.exec();
}