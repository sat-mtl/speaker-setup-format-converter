#include "mainwindow.hpp"

#include <QApplication>
#include <QFile>
#include <QFontDatabase>

int main(int argc, char* argv[])
{
  // Debug log
  qputenv("QT_ASSUME_STDERR_HAS_CONSOLE", "1");

  // Make sure numbers are parsed with dots
  qputenv("LC_ALL", "C.UTF-8");
  QLocale::setDefault(QLocale::C);
  setlocale(LC_ALL, "C.UTF-8");

  QApplication app(argc, argv);
  Q_INIT_RESOURCE(fonts);
  QFontDatabase::addApplicationFont(":/fonts/DMSans-VariableFont_opsz,wght.ttf");
  QFontDatabase::addApplicationFont(":/fonts/DMSans-Italic-VariableFont_opsz,wght.ttf");

  QFont font("DM Sans");
  font.setWeight(QFont::Medium);
  font.setHintingPreference(QFont::HintingPreference::PreferVerticalHinting);
  QApplication::setFont(font);
  QApplication::setApplicationName("Speaker Layout Converter");
  QApplication::setOrganizationName("Société des Arts Technologiques");
  QApplication::setOrganizationDomain("sat.qc.ca");

  MainWindow window;
  window.show();

  QLocale::setDefault(QLocale::C);
  setlocale(LC_ALL, "C");

  return app.exec();
}
