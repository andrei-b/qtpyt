#pragma once
#include <QMainWindow>
#include <memory>

namespace qtpyt { class QPyModule; }

class QTextEdit;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onApplyScript();

private:
    void appendLog(const QString& s);
    void applyScript(const QString& py);

    QTextEdit* editor_ = nullptr;
    QTextEdit* log_ = nullptr;

    // keep last module alive (optional)
    std::unique_ptr<qtpyt::QPyModule> module_;
};
