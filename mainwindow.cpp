#include "mainwindow.hpp"

#include "aiira_parser.hpp"
#include "converter.hpp"
#include "csv_parser.hpp"
#include "ease_parser.hpp"
#include "spat_parser.hpp"
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

MainWindow::MainWindow(QWidget *parent)
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
    m_inputFormatCombo->addItems({"Auto-detect", "EASE", "AIIRA", "SPAT", "CSV", "SpeakerView"});
    connect(m_inputFormatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onInputFormatChanged);
    formatLayout->addWidget(m_inputFormatCombo);

    mainLayout->addLayout(formatLayout);
    
    // Input area
    auto* inputLabel = new QLabel("Input:");
    mainLayout->addWidget(inputLabel);
    
    m_inputTextEdit = new QTextEdit();
    m_inputTextEdit->setMinimumHeight(200);
    mainLayout->addWidget(m_inputTextEdit);
    
    // Buttons
    auto* buttonLayout = new QHBoxLayout();

    buttonLayout->addWidget(new QLabel("Output Format:"));
    m_outputFormatCombo = new QComboBox();
    m_outputFormatCombo->addItems({"EASE", "AIIRA", "SPAT", "CSV", "SpeakerView"});
    m_outputFormatCombo->setCurrentIndex(0);
    buttonLayout->addWidget(m_outputFormatCombo);
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
    connect(m_inputTextEdit, &QTextEdit::textChanged, this, &MainWindow::updateConvertButton);
}

void MainWindow::onLoadFile()
{
  QString filter
      = "All Supported (*.xld *.ease *.json *.rtf *.csv *.xml);;EASE Files (*.xld "
        "*.ease);;AIIRA Files (*.json);;SPAT Files (*.rtf);;CSV Files "
        "(*.csv);;SpeakerView Files (*.xml);;All Files (*)";
  QString filePath = QFileDialog::getOpenFileName(
      this, "Load Speaker Configuration", QString(), filter);

  if(filePath.isEmpty())
  {
    return;
  }

  QFile file(filePath);
  if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    QMessageBox::critical(this, "Error", "Could not open file: " + file.errorString());
    return;
  }
    
    QTextStream stream(&file);
    QString content = stream.readAll();
    m_inputTextEdit->setPlainText(content);
    m_currentFilePath = filePath;
    
    // Auto-detect format if needed
    if (m_inputFormatCombo->currentIndex() == 0) {
        std::string detectedFormat = detectFormat(filePath);
        if (!detectedFormat.empty()) {
            for (int i = 1; i < m_inputFormatCombo->count(); ++i) {
                if (m_inputFormatCombo->itemText(i).toLower() == QString::fromStdString(detectedFormat).toLower()) {
                    m_inputFormatCombo->setCurrentIndex(i);
                    break;
                }
            }
        }
    }
    
    m_statusLabel->setText("Loaded: " + QFileInfo(filePath).fileName());
}

void MainWindow::onSaveFile()
{
    QString filter = "SpeakerView Files (*.xml);;All Files (*)";
    QString filePath = QFileDialog::getSaveFileName(this, "Save Converted File", QString(), filter);
    
    if (filePath.isEmpty()) {
        return;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Could not save file: " + file.errorString());
        return;
    }
    
    QTextStream stream(&file);
    stream << m_outputTextEdit->toPlainText();
    
    m_statusLabel->setText("Saved: " + QFileInfo(filePath).fileName());
}

void MainWindow::onConvert()
{
    m_inputContent = m_inputTextEdit->toPlainText().toStdString();
    
    if (m_inputContent.empty()) {
        QMessageBox::warning(this, "Warning", "Input is empty");
        return;
    }
    
    std::string inputFormat = m_inputFormatCombo->currentText().toStdString();
    std::string outputFormat = m_outputFormatCombo->currentText().toStdString();

    // If auto-detect, try to detect from content
    if (inputFormat == "Auto-detect") {
        if (!m_currentFilePath.isEmpty()) {
            inputFormat = detectFormat(m_currentFilePath);
        }
        if (inputFormat.empty()) {
            QMessageBox::warning(this, "Warning", "Could not detect input format. Please select manually.");
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
      else if(inputFormat == "AIIRA")
      {
        if(auto parsed = spatparse::aiira::parse(m_inputContent))
        {
          unified_config.emplace();
          convert(*parsed, *unified_config);
        }
      }
      else if(inputFormat == "SPAT")
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
      else if(inputFormat == "SpeakerView")
      {
        if(auto parsed = spatparse::speakerview::parse(m_inputContent))
        {
          unified_config.emplace();
          convert(*parsed, *unified_config);
        }
      }

      // Step 2: Check for parsing success and serialize from the unified format.
      if(unified_config)
      {
        std::string output_string;

        if(outputFormat == "EASE")
        {
          spatparse::ease::file f;
          convert(*unified_config, f);
          output_string = ease::to_string(f);
        }
        else if(outputFormat == "AIIRA")
        {
          spatparse::aiira::file f;
          convert(*unified_config, f);
          output_string = to_string(f);
        }
        else if(outputFormat == "SPAT")
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
        else if(outputFormat == "SpeakerView")
        {
          spatparse::speakerview::file f;
          convert(*unified_config, f);
          output_string = to_string(f); // This uses your existing target format
        }

        // Update the UI with the result
        m_outputTextEdit->setPlainText(QString::fromStdString(output_string));
        m_saveButton->setEnabled(true);
        m_statusLabel->setText(
            QString("Conversion from %1 to %2 complete").arg(inputFormat, outputFormat));
      }
      else
      {
        // Parsing failed
        QMessageBox::critical(
            this, "Error",
            QString("Failed to parse input file as %1 format.").arg(inputFormat));
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
}

void MainWindow::updateConvertButton()
{
    m_convertButton->setEnabled(!m_inputTextEdit->toPlainText().isEmpty());
}

std::string MainWindow::detectFormat(const QString& filePath)
{
    QFileInfo info(filePath);
    QString ext = info.suffix().toLower();
    
    if (ext == "ease") return "EASE";
    if (ext == "json") return "AIIRA";
    if (ext == "rtf") return "SPAT";
    if (ext == "csv") return "CSV";
    if (ext == "xml") return "SpeakerView";
    
    return "";
}
