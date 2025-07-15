#pragma once

#include <QMainWindow>

#include <memory>

QT_BEGIN_NAMESPACE
class QComboBox;
class QTextEdit;
class QPushButton;
class QLabel;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget* parent = nullptr);
  ~MainWindow();

private slots:
  void onLoadFile();
  void doLoadFile(const QString& fileName, const QByteArray& fileContent);
  void onSaveFile();
  void onConvert();
  void onInputFormatChanged(int index);

private:
  void setupUi();
  void updateConvertButton();
  std::string detectFormat(const QString& filePath);

  QComboBox* m_inputFormatCombo;
  QComboBox* m_outputFormatCombo;
  QTextEdit* m_inputTextEdit;
  QTextEdit* m_outputTextEdit;
  QPushButton* m_loadButton;
  QPushButton* m_saveButton;
  QPushButton* m_convertButton;
  QLabel* m_statusLabel;

  QString m_currentFilePath;
  std::string m_inputContent;
};
