#include "mainwindow.h"
#include "appearanceapi.h"

#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QMessageBox>

#include "../../../Qt/6.10.1/gcc_64/include/QtCore/QMetaType"
#include "../../../Qt/6.10.1/gcc_64/include/QtCore/QObject"
#include "../../include/qtpyt/qpymodule.h"
#include "qtpyt/qpymodule.h"      // uses your uploaded API
#include "qtpyt/qpymodulebase.h"  // (included by qpymodule.h anyway)

static QString defaultScript() {
    // This script:
    //  - installs stdout redirection to qt_log
    //  - defines configure(api) and calls it
    return R"PY(
import sys, time

class _Stdout:
    def __init__(self, cb):
        self.cb = cb
    def write(self, s):
        if s:
            self.cb(str(s))
    def flush(self): pass


def configure(api):
    sys.stdout = _Stdout(qt_log)
    sys.stderr = _Stdout(qt_log)
    print("Configuring window from Python…")
    api.set_title("Configured by Python 🐍")
    api.resize(980, 620)
    api.set_opacity(0.96)

    # nice dark theme example
    api.set_stylesheet("""
        QMainWindow { background: #121212; }
        QTextEdit { background: #1b1b1b; color: #eaeaea; border: 1px solid #2a2a2a; }
        QPushButton { padding: 8px 12px; }
    """)

    api.set_font("DejaVu Sans", 11, False)
    api.set_bg("#121212")
)PY";
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {

    auto* central = new QWidget(this);
    setCentralWidget(central);

    editor_ = new QTextEdit(this);
    editor_->setPlainText(defaultScript());

    log_ = new QTextEdit(this);
    log_->setReadOnly(true);

    auto* applyBtn = new QPushButton("Apply Python script", this);

    auto* topRow = new QHBoxLayout();
    topRow->addWidget(applyBtn);
    topRow->addStretch(1);

    auto* splitter = new QSplitter(this);
    splitter->addWidget(editor_);
    splitter->addWidget(log_);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 1);

    auto* layout = new QVBoxLayout(central);
    layout->addLayout(topRow);
    layout->addWidget(splitter);

    connect(applyBtn, &QPushButton::clicked, this, &MainWindow::onApplyScript);

    setWindowTitle("Appearance-by-Python (qtpyt)");
    resize(1000, 650);

    appendLog("Ready. Edit the script and click Apply.");
}

MainWindow::~MainWindow() = default;

void MainWindow::appendLog(const QString& s) {
    log_->append(s);
}

void MainWindow::onApplyScript() {
    applyScript(editor_->toPlainText());
}

void MainWindow::applyScript(const QString& py) {
    try {
        // Rebuild the module from source each time (simple & predictable).
        module_ = std::make_unique<qtpyt::QPyModule>(py, qtpyt::QPySourceType::SourceString);
        if (!module_->isValid()) {
            appendLog("Module invalid (build failed).");
            return;
        }

        // Create the API object exposed to Python
        auto* apiObj = new AppearanceApi(this, this);

        // Expose objects
        module_->addVariable("api", apiObj);

        // Redirect python output into the log panel:
        // qtpyt supports exposing C++ functions via addFunction(name, std::function<...>)
        module_->addFunction<void, QString>("qt_log",
            std::function<void(QString)>([this](const QString& s) {
                // safe: called in whatever thread Python is running;
                // we marshal to UI thread via queued invoke.
                QMetaObject::invokeMethod(this, [this, s]{
                    appendLog(s.trimmed());
                }, Qt::QueuedConnection);
            })
        );

        // The script already calls configure(api) at the bottom.
        // But if you prefer calling explicitly, you could do:
        // module_->call<void>("configure", apiObj);
        module_->callAsync<QObject*>(nullptr,"configure", QMetaType::Void, apiObj);
        appendLog("Applied.");
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Python error", e.what());
        appendLog(QString("Exception: %1").arg(e.what()));
    }
}
