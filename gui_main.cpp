#include "mainwindow.hpp"
#include <QApplication>

int main(int argc, char *argv[])
{
  // Make sure numbers are parsed with dots
  qputenv("LC_ALL", "C");
  QLocale::setDefault(QLocale::C);
  setlocale(LC_ALL, "C");

  QApplication app(argc, argv);

  QApplication::setApplicationName("Spatial Format Converter");
  QApplication::setOrganizationName("Société des Arts Technologiques");
  QApplication::setOrganizationDomain("sat.qc.ca");

  MainWindow window;
  window.show();

  QLocale::setDefault(QLocale::C);
  setlocale(LC_ALL, "C");

  return app.exec();
}
