#include "mainwindow.hpp"

#include "aiira_parser.hpp"
#include "converter.hpp"
#include "csv_parser.hpp"
#include "ease_parser.hpp"
#include "fourdsound_parser.hpp"
#include "spat_parser.hpp"
#include "spat_revolution_parser.hpp"
#include "speakerview_parser.hpp"

#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStatusBar>
#include <QTextEdit>
#include <QTextStream>
#include <QVBoxLayout>

#include <fstream>
#include <sstream>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
  setupUi();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi()
{
  auto* centralWidget = new QWidget(this);
  setCentralWidget(centralWidget);

  auto* mainLayout = new QVBoxLayout(centralWidget);

  // Format selection area
  auto* formatLayout = new QHBoxLayout();
  m_loadButton = new QPushButton("Load File...");
  connect(m_loadButton, &QPushButton::clicked, this, &MainWindow::onLoadFile);
  formatLayout->addWidget(m_loadButton);
  formatLayout->addWidget(new QLabel("Input Format:"));

  m_inputFormatCombo = new QComboBox();
  m_inputFormatCombo->addItems(
      {"Auto-detect", "EASE", "AIIRA (IEM)", "Spat (IRCAM)", "CSV", "SpatGRIS",
       "4D Sound", "Spat Revolution"});
  connect(
      m_inputFormatCombo, qOverload<int>(&QComboBox::currentIndexChanged), this,
      &MainWindow::onInputFormatChanged);
  formatLayout->addWidget(m_inputFormatCombo);

  mainLayout->addLayout(formatLayout);

  // Input area
  auto* inputLabel = new QLabel("Input:");
  mainLayout->addWidget(inputLabel);

  m_inputTextEdit = new QTextEdit();
  m_inputTextEdit->setAcceptRichText(false);
  m_inputTextEdit->setAutoFormatting(QTextEdit::AutoNone);
  m_inputTextEdit->setMinimumHeight(200);
  mainLayout->addWidget(m_inputTextEdit);

  // Buttons
  auto* buttonLayout = new QHBoxLayout();

  buttonLayout->addWidget(new QLabel("Output Format:"));
  m_outputFormatCombo = new QComboBox();
  m_outputFormatCombo->addItems(
      {"EASE", "IEM", "Spat (IRCAM)", "CSV", "SpatGRIS (v3)", "SpatGRIS (v4)",
       "4D Sound", "Spat Revolution"});
  m_outputFormatCombo->setCurrentIndex(0);
  connect(
      m_outputFormatCombo, qOverload<int>(&QComboBox::currentIndexChanged), this,
      &MainWindow::onOutputFormatChanged);
  buttonLayout->addWidget(m_outputFormatCombo);

  buttonLayout->addWidget(new QLabel{"Conversion distance ratio:"});
  m_conversionRatio = new QDoubleSpinBox;
  m_conversionRatio->setEnabled(false);
  m_conversionRatio->setRange(0.0, 1e9);
  m_conversionRatio->setValue(1.0);
  buttonLayout->addWidget(m_conversionRatio);

  m_convertButton = new QPushButton("Convert");
  m_convertButton->setEnabled(false);
  connect(m_convertButton, &QPushButton::clicked, this, &MainWindow::onConvert);
  buttonLayout->addWidget(m_convertButton);

  buttonLayout->addStretch();
  mainLayout->addLayout(buttonLayout);

  // Output area
  auto* outputLabel = new QLabel("Output:");
  mainLayout->addWidget(outputLabel);

  m_outputTextEdit = new QTextEdit();
  m_outputTextEdit->setMinimumHeight(200);
  m_outputTextEdit->setReadOnly(true);
  mainLayout->addWidget(m_outputTextEdit);

  // Save button
  auto* saveLayout = new QHBoxLayout();
  m_saveButton = new QPushButton("Save Output...");
  m_saveButton->setEnabled(false);
  connect(m_saveButton, &QPushButton::clicked, this, &MainWindow::onSaveFile);
  saveLayout->addWidget(m_saveButton);
  saveLayout->addStretch();
  mainLayout->addLayout(saveLayout);

  // Status bar
  m_statusLabel = new QLabel("Ready");
  statusBar()->addWidget(m_statusLabel);

  // Window properties
  setWindowTitle("Spatial Format Converter");
  resize(800, 600);

  // Connect text changes to update convert button
  connect(
      m_inputTextEdit, &QTextEdit::textChanged, this, &MainWindow::updateConvertButton);
}

void MainWindow::onLoadFile()
{
  QString filter
      = "All Supported (*.xld *.ease *.json *.rtf *.csv *.xml *.ioconfig);;EASE Files "
        "(*.xld "
        "*.ease);;AIIRA Files (*.json);;SPAT Files (*.rtf);;CSV Files "
        "(*.csv);;SpeakerView Files (*.xml);;4D Sound Files (*.xml);;Spat Revolution "
        "Files (*.ioconfig);;All Files (*)";

  QFileDialog::getOpenFileContent(
      filter, [this](const QString& fileName, const QByteArray& fileContent) {
    if(!fileName.isEmpty())
    {
      doLoadFile(fileName, fileContent);
    }
  });
}

void MainWindow::doLoadFile(const QString& filePath, const QByteArray& fileContent)
{
  if(filePath.isEmpty())
    return;

  QString content = fileContent;
  m_inputTextEdit->setPlainText(content);
  m_currentFilePath = filePath;

  m_statusLabel->setText("Loaded: " + QFileInfo(filePath).fileName());

  updateDistanceRatioWidget();
}
void MainWindow::onSaveFile()
{
  std::string outputFormat = m_outputFormatCombo->currentText().toStdString();
  QString hint;

  if(outputFormat == "EASE")
  {
    hint = "speakers.xld";
  }
  else if(outputFormat == "AIIRA (IEM)")
  {
    hint = "speakers.json";
  }
  else if(outputFormat == "Spat (IRCAM)")
  {
    hint = "speakers.rtf";
  }
  else if(outputFormat == "CSV")
  {
    hint = "speakers.csv";
  }
  else if(outputFormat.starts_with("SpatGRIS"))
  {
    hint = "speakers.xml";
  }
  else if(outputFormat == "Spat Revolution")
  {
    hint = "speakers.ioconfig";
  }

  QFileDialog::saveFileContent(m_outputTextEdit->toPlainText().toUtf8(), hint);

  m_statusLabel->setText("Saved!");
}

void MainWindow::onConvert()
{
  m_inputContent = m_inputTextEdit->toPlainText().toStdString();

  if(m_inputContent.empty())
  {
    QMessageBox::warning(this, "Warning", "Input is empty");
    return;
  }

  std::string inputFormat = m_inputFormatCombo->currentText().toStdString();
  std::string outputFormat = m_outputFormatCombo->currentText().toStdString();

  // If auto-detect, try to detect from content
  if(inputFormat == "Auto-detect")
  {
    if(!m_currentFilePath.isEmpty())
    {
      inputFormat = detectFormat(m_currentFilePath);
    }
    if(inputFormat.empty())
    {
      QMessageBox::warning(
          this, "Warning", "Could not detect input format. Please select manually.");
      return;
    }
  }

  try
  {
    using namespace spatparse;
    // Step 1: Parse the input string into the unified format.
    std::optional<spatparse::unified::loudspeaker_configuration> unified_config;

    if(inputFormat == "EASE")
    {
      if(auto parsed = spatparse::ease::parse(m_inputContent))
      {
        unified_config.emplace();
        convert(*parsed, *unified_config);
      }
    }
    else if(inputFormat == "IEM")
    {
      if(auto parsed = spatparse::aiira::parse(m_inputContent))
      {
        unified_config.emplace();
        convert(*parsed, *unified_config);
      }
    }
    else if(inputFormat == "Spat (IRCAM)")
    {
      if(auto parsed = spatparse::spat::parse(m_inputContent))
      {
        unified_config.emplace();
        convert(*parsed, *unified_config);
      }
    }
    else if(inputFormat == "CSV")
    {
      if(auto parsed = spatparse::csv::parse(m_inputContent))
      {
        unified_config.emplace();
        convert(*parsed, *unified_config);
      }
    }
    else if(inputFormat.starts_with("SpatGRIS"))
    {
      if(auto parsed = spatparse::spatgris::parse(m_inputContent))
      {
        unified_config.emplace();
        convert(*parsed, *unified_config);
      }
    }
    else if(inputFormat == "4D Sound")
    {
      if(auto parsed = spatparse::fourdsound::parse(m_inputContent))
      {
        unified_config.emplace();
        convert(*parsed, *unified_config);
      }
    }
    else if(inputFormat == "Spat Revolution")
    {
      if(auto parsed = spatparse::spat_revolution::parse(m_inputContent))
      {
        unified_config.emplace();
        convert(*parsed, *unified_config);
      }
    }

    QString qinputFormat  = QString::fromStdString(inputFormat);
    QString qoutputFormat = QString::fromStdString(outputFormat);

    // Step 2: Check for parsing success and serialize from the unified format.
    if(unified_config)
    {
      if(this->m_conversionRatio->isEnabled())
      {
        unified_config->normalization_ratio = this->m_conversionRatio->value();
      }
      std::string output_string;

      if(outputFormat == "EASE")
      {
        spatparse::ease::file f;
        convert(*unified_config, f);
        output_string = ease::to_string(f);
      }
      else if(outputFormat == "IEM")
      {
        spatparse::aiira::file f;
        convert(*unified_config, f);
        output_string = to_string(f);
      }
      else if(outputFormat == "Spat (IRCAM)")
      {
        spatparse::spat::file f;
        convert(*unified_config, f);
        output_string = to_string(f);
      }
      else if(outputFormat == "CSV")
      {
        spatparse::csv::file f;
        convert(*unified_config, f);
        output_string = to_string(f);
      }
      else if(outputFormat == "SpatGRIS (v3)")
      {
        spatparse::spatgris::file f;
        convert(*unified_config, f);
        output_string = to_string(f);
      }
      else if(outputFormat == "SpatGRIS (v4)")
      {
        spatparse::spatgris::file f;
        convert(*unified_config, f);
        output_string = to_string_v4(f);
      }
      else if(outputFormat == "4D Sound")
      {
        spatparse::fourdsound::file f;
        convert(*unified_config, f);
        output_string = to_string(f);
      }

      // Update the UI with the result
      m_outputTextEdit->setPlainText(QString::fromStdString(output_string));
      m_saveButton->setEnabled(true);
      m_statusLabel->setText(
          QString("Conversion from %1 to %2 complete").arg(qinputFormat).arg(qoutputFormat));
    }
    else
    {
      // Parsing failed
      QMessageBox::critical(
          this, "Error",
          QString("Failed to parse input file as %1 format.").arg(qinputFormat));
      m_statusLabel->setText("Conversion failed");
    }
  }
  catch(const std::exception& e)
  {
    QMessageBox::critical(
        this, "Error", QString("An unexpected error occurred: %1").arg(e.what()));
    m_statusLabel->setText("Conversion failed");
  }
}

void MainWindow::onInputFormatChanged(int index)
{
  updateConvertButton();
  updateDistanceRatioWidget();
}

void MainWindow::onOutputFormatChanged(int index)
{
  updateDistanceRatioWidget();
}
void MainWindow::updateConvertButton()
{
  m_convertButton->setEnabled(!m_inputTextEdit->toPlainText().isEmpty());
}

void MainWindow::updateDistanceRatioWidget()
{
  this->m_conversionRatio->setEnabled(false);
  std::string in_format = m_inputFormatCombo->currentText().toStdString();
  if(m_inputFormatCombo->currentIndex() == 0)
  {
    // Auto-detect
    in_format = detectFormat(m_currentFilePath);
  }
  std::string out_format = m_outputFormatCombo->currentText().toStdString();
  if(in_format == out_format)
    return;
  if(in_format.contains("SpatGRIS"))
  {
    if(out_format.contains("SpatGRIS"))
      this->m_conversionRatio->setEnabled(false);
    else
      this->m_conversionRatio->setEnabled(true);
  }
}

std::string MainWindow::detectFormat(const QString& filePath)
{
  QFileInfo info(filePath);
  QString ext = info.suffix().toLower();

  if(ext == "ease")
    return "EASE";
  if(ext == "json")
    return "IEM";
  if(ext == "rtf")
    return "Spat (IRCAM)";
  if(ext == "csv")
    return "CSV";
  if(ext == "ioconfig")
    return "Spat Revolution";
  if(ext == "xml")
  {
    // Need to check content to distinguish between SpatGRIS and 4D Sound
    QFile file(filePath);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      QTextStream stream(&file);
      QString content = stream.read(100);
      if(content.contains("<setup"))
        return "4D Sound";
      else if(content.contains("<SPEAKER_SETUP"))
        return "SpatGRIS";
    }
    return "SpatGRIS"; // Default to SpatGRIS for XML
  }

  return "";
}
