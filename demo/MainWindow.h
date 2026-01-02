#pragma once

#include <QMainWindow>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <memory>

namespace qtpyt {
    class PyRunner;
}

/**
 * @brief MainWindow class for the qtpyt demo application.
 * 
 * Provides a GUI interface for executing Python scripts using the qtpyt library.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onInitializePython();
    void onExecuteScript();
    void onClearOutput();
    void onLoadExample();
    void onClear();
    void onAbout();
    void onInitializationChanged(bool initialized);
    void onScriptExecuted(bool success, const QString& message);

private:
    void setupUi();
    void setupMenuBar();
    void createConnections();
    void updateStatus();
    void loadExampleScript(const QString& filename);
    QString readExampleFile(const QString& filename);

    // UI Components
    QPlainTextEdit* m_scriptEditor;
    QPushButton* m_initButton;
    QPushButton* m_executeButton;
    QPushButton* m_clearButton;
    QTextEdit* m_outputArea;
    QLabel* m_statusLabel;

    // PyRunner instance
    std::unique_ptr<qtpyt::PyRunner> m_pyRunner;
};
