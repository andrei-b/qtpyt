#include "MainWindow.h"
#include <qtpyt/PyRunner.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>
#include <QInputDialog>
#include <QFont>
#include <QSplitter>
#include <QGroupBox>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QCoreApplication>
#include <QFileInfo>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_scriptEditor(nullptr)
    , m_initButton(nullptr)
    , m_executeButton(nullptr)
    , m_clearButton(nullptr)
    , m_outputArea(nullptr)
    , m_statusLabel(nullptr)
    , m_pyRunner(std::make_unique<qtpyt::PyRunner>(this))
{
    setupUi();
    setupMenuBar();
    createConnections();
    updateStatus();
    
    // Set window properties
    setWindowTitle("qtpyt Demo - Python Runner");
    resize(900, 700);
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi() {
    // Create central widget
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // Main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    
    // Create splitter for editor and output
    QSplitter* splitter = new QSplitter(Qt::Vertical, this);
    
    // === Script Editor Section ===
    QGroupBox* editorGroup = new QGroupBox("Python Script Editor", this);
    QVBoxLayout* editorLayout = new QVBoxLayout(editorGroup);
    
    m_scriptEditor = new QPlainTextEdit(this);
    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    font.setPointSize(10);
    m_scriptEditor->setFont(font);
    m_scriptEditor->setPlaceholderText("Enter Python code here or load an example from the File menu...");
    m_scriptEditor->setLineWrapMode(QPlainTextEdit::NoWrap);
    editorLayout->addWidget(m_scriptEditor);
    
    splitter->addWidget(editorGroup);
    
    // === Output Section ===
    QGroupBox* outputGroup = new QGroupBox("Output", this);
    QVBoxLayout* outputLayout = new QVBoxLayout(outputGroup);
    
    m_outputArea = new QTextEdit(this);
    m_outputArea->setReadOnly(true);
    m_outputArea->setFont(font);
    outputLayout->addWidget(m_outputArea);
    
    splitter->addWidget(outputGroup);
    
    // Set splitter sizes (60% editor, 40% output)
    splitter->setStretchFactor(0, 60);
    splitter->setStretchFactor(1, 40);
    
    mainLayout->addWidget(splitter);
    
    // === Control Buttons Section ===
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    m_initButton = new QPushButton("Initialize Python", this);
    m_initButton->setToolTip("Initialize the Python interpreter");
    m_initButton->setMinimumHeight(35);
    
    m_executeButton = new QPushButton("Execute Script", this);
    m_executeButton->setToolTip("Execute the Python script in the editor");
    m_executeButton->setEnabled(false);
    m_executeButton->setMinimumHeight(35);
    
    m_clearButton = new QPushButton("Clear Output", this);
    m_clearButton->setToolTip("Clear the output area");
    m_clearButton->setMinimumHeight(35);
    
    buttonLayout->addWidget(m_initButton);
    buttonLayout->addWidget(m_executeButton);
    buttonLayout->addWidget(m_clearButton);
    buttonLayout->addStretch();
    
    mainLayout->addLayout(buttonLayout);
    
    // === Status Bar ===
    m_statusLabel = new QLabel("Status: Not initialized", this);
    statusBar()->addPermanentWidget(m_statusLabel);
    
    // Add some styling
    setStyleSheet(R"(
        QPushButton {
            padding: 5px 15px;
            font-weight: bold;
        }
        QPushButton:disabled {
            color: #888;
        }
        QGroupBox {
            font-weight: bold;
            border: 2px solid #ccc;
            border-radius: 5px;
            margin-top: 10px;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px;
        }
    )");
}

void MainWindow::setupMenuBar() {
    // File menu
    QMenu* fileMenu = menuBar()->addMenu("&File");
    
    QAction* loadExampleAction = fileMenu->addAction("Load &Example...");
    loadExampleAction->setShortcut(QKeySequence("Ctrl+E"));
    connect(loadExampleAction, &QAction::triggered, this, &MainWindow::onLoadExample);
    
    QAction* clearAction = fileMenu->addAction("&Clear Script");
    clearAction->setShortcut(QKeySequence("Ctrl+L"));
    connect(clearAction, &QAction::triggered, this, &MainWindow::onClear);
    
    fileMenu->addSeparator();
    
    QAction* exitAction = fileMenu->addAction("E&xit");
    exitAction->setShortcut(QKeySequence("Ctrl+Q"));
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    
    // Help menu
    QMenu* helpMenu = menuBar()->addMenu("&Help");
    
    QAction* aboutAction = helpMenu->addAction("&About");
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::createConnections() {
    connect(m_initButton, &QPushButton::clicked, this, &MainWindow::onInitializePython);
    connect(m_executeButton, &QPushButton::clicked, this, &MainWindow::onExecuteScript);
    connect(m_clearButton, &QPushButton::clicked, this, &MainWindow::onClearOutput);
    
    connect(m_pyRunner.get(), &qtpyt::PyRunner::initializationChanged, 
            this, &MainWindow::onInitializationChanged);
    connect(m_pyRunner.get(), &qtpyt::PyRunner::scriptExecuted,
            this, &MainWindow::onScriptExecuted);
}

void MainWindow::onInitializePython() {
    m_outputArea->append("=== Initializing Python interpreter ===");
    
    if (m_pyRunner->initialize()) {
        m_outputArea->append("✓ Python interpreter initialized successfully");
    } else {
        m_outputArea->append("✗ Failed to initialize Python interpreter");
        m_outputArea->append("Error: " + m_pyRunner->getLastError());
    }
    
    m_outputArea->append("");
}

void MainWindow::onExecuteScript() {
    QString script = m_scriptEditor->toPlainText();
    
    if (script.trimmed().isEmpty()) {
        QMessageBox::warning(this, "Empty Script", 
                           "Please enter some Python code to execute.");
        return;
    }
    
    m_outputArea->append("=== Executing Python script ===");
    
    // Redirect Python stdout to capture print statements
    QString captureScript = R"(
import sys
from io import StringIO

_qtpyt_stdout = StringIO()
_qtpyt_old_stdout = sys.stdout
sys.stdout = _qtpyt_stdout

try:
)" + script + R"(
finally:
    sys.stdout = _qtpyt_old_stdout
    _output = _qtpyt_stdout.getvalue()
    if _output:
        print(_output, end='')
)";
    
    if (m_pyRunner->executeScript(captureScript)) {
        m_outputArea->append("✓ Script executed successfully");
    } else {
        m_outputArea->append("✗ Script execution failed");
        m_outputArea->append("Error: " + m_pyRunner->getLastError());
    }
    
    m_outputArea->append("");
}

void MainWindow::onClearOutput() {
    m_outputArea->clear();
    m_outputArea->append("Output cleared.");
}

void MainWindow::onLoadExample() {
    QStringList examples;
    examples << "Hello World" << "Mathematical Operations" << "Data Structures";
    
    bool ok;
    QString selected = QInputDialog::getItem(this, "Load Example",
                                            "Select an example script:",
                                            examples, 0, false, &ok);
    
    if (ok && !selected.isEmpty()) {
        QString filename;
        if (selected == "Hello World") {
            filename = "hello.py";
        } else if (selected == "Mathematical Operations") {
            filename = "math_demo.py";
        } else if (selected == "Data Structures") {
            filename = "data_structures.py";
        }
        
        loadExampleScript(filename);
    }
}

void MainWindow::onClear() {
    m_scriptEditor->clear();
}

void MainWindow::onAbout() {
    QMessageBox::about(this, "About qtpyt Demo",
        "<h2>qtpyt Demo Application</h2>"
        "<p>This application demonstrates the qtpyt library's Python integration capabilities.</p>"
        "<p><b>qtpyt</b> allows you to embed multi-threaded Python into Qt applications.</p>"
        "<p><b>Features:</b></p>"
        "<ul>"
        "<li>Execute Python scripts from Qt</li>"
        "<li>Access Qt objects from Python</li>"
        "<li>Signal/slot integration</li>"
        "<li>Asynchronous Python execution</li>"
        "</ul>"
        "<p><b>Version:</b> 0.0.1</p>"
        "<p>Built with Qt " + QString(QT_VERSION_STR) + "</p>");
}

void MainWindow::onInitializationChanged(bool initialized) {
    m_executeButton->setEnabled(initialized);
    updateStatus();
}

void MainWindow::onScriptExecuted(bool success, const QString& message) {
    // This is handled in onExecuteScript
    Q_UNUSED(success);
    Q_UNUSED(message);
}

void MainWindow::updateStatus() {
    if (m_pyRunner->isInitialized()) {
        m_statusLabel->setText("Status: ✓ Python initialized");
        m_statusLabel->setStyleSheet("QLabel { color: green; font-weight: bold; }");
        m_initButton->setEnabled(false);
    } else {
        m_statusLabel->setText("Status: ✗ Not initialized");
        m_statusLabel->setStyleSheet("QLabel { color: red; font-weight: bold; }");
        m_initButton->setEnabled(true);
    }
}

void MainWindow::loadExampleScript(const QString& filename) {
    QString content = readExampleFile(filename);
    
    if (!content.isEmpty()) {
        m_scriptEditor->setPlainText(content);
        m_outputArea->append("Loaded example: " + filename);
    } else {
        QMessageBox::warning(this, "Load Failed",
                           "Could not load example file: " + filename);
    }
}

QString MainWindow::readExampleFile(const QString& filename) {
    // Try multiple possible locations for the example files
    QStringList searchPaths;
    searchPaths << QDir::currentPath() + "/demo/examples/" + filename;
    searchPaths << QDir::currentPath() + "/examples/" + filename;
    searchPaths << QCoreApplication::applicationDirPath() + "/examples/" + filename;
    searchPaths << QCoreApplication::applicationDirPath() + "/../demo/examples/" + filename;
    searchPaths << QCoreApplication::applicationDirPath() + "/../share/qtpyt_demo/examples/" + filename;
    
    // Also try relative to the source directory (for development)
    QString sourceDir = QStringLiteral(__FILE__);
    sourceDir = QFileInfo(sourceDir).absolutePath() + "/examples/" + filename;
    searchPaths.prepend(sourceDir);
    
    for (const QString& path : searchPaths) {
        QFile file(path);
        if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString content = in.readAll();
            file.close();
            return content;
        }
    }
    
    return QString();
}
