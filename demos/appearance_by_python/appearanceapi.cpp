#include "appearanceapi.h"
#include <QWidget>
#include <QPalette>

AppearanceApi::AppearanceApi(QWidget* targetWindow, QObject* parent)
    : QObject(parent), w_(targetWindow) {}

void AppearanceApi::set_title(const QString& title) {
    if (!w_) return;
    w_->setWindowTitle(title);
}

void AppearanceApi::set_stylesheet(const QString& css) {
    if (!w_) return;
    w_->setStyleSheet(css);
}

void AppearanceApi::set_bg(const QString& colorNameOrHex) {
    if (!w_) return;

    QColor c(colorNameOrHex);
    if (!c.isValid()) return;

    // For QWidget backgrounds, easiest is stylesheet:
    w_->setStyleSheet(w_->styleSheet() + QString("\nQWidget { background: %1; }\n").arg(c.name()));
}

void AppearanceApi::set_font(const QString& family, int pointSize, bool bold) {
    if (!w_) return;
    QFont f(family, pointSize);
    f.setBold(bold);
    w_->setFont(f);
}

void AppearanceApi::set_opacity(double opacity01) {
    if (!w_) return;
    if (opacity01 < 0.05) opacity01 = 0.05;
    if (opacity01 > 1.0)  opacity01 = 1.0;
    w_->setWindowOpacity(opacity01);
}

void AppearanceApi::resize(int w, int h) {
    if (!w_) return;
    w_->resize(w, h);
}

void AppearanceApi::move(int x, int y) {
    if (!w_) return;
    w_->move(x, y);
}
