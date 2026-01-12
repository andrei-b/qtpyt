#pragma once
#include <QObject>
#include <QColor>
#include <QFont>

class QWidget;

class AppearanceApi : public QObject {
    Q_OBJECT
public:
    explicit AppearanceApi(QWidget* targetWindow, QObject* parent = nullptr);

public slots:
    void set_title(const QString& title);
    void set_stylesheet(const QString& css);
    void set_bg(const QString& colorNameOrHex);
    void set_font(const QString& family, int pointSize, bool bold = false);
    void set_opacity(double opacity01);
    void resize(int w, int h);
    void move(int x, int y);

private:
    QWidget* w_ = nullptr;
};
