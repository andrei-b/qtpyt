#pragma once

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QDateTime>

class TimePanel final : public QWidget
{
    Q_OBJECT
public:
    explicit TimePanel(QWidget* parent = nullptr);

    Q_INVOKABLE void setTimeFormat(const QString& fmt);
    QString timeFormat() const { return m_format; }

    Q_INVOKABLE void setTimeZoneId(const QByteArray& tz); // e.g. "Europe/London", or empty for local
    QByteArray timeZoneId() const { return m_tzId; }

    QLabel* label() const { return m_label; }

private slots:
    void updateNow();

private:
    QLabel* m_label = nullptr;
    QTimer  m_timer;
    QString m_format = "HH:mm:ss";
    QByteArray m_tzId; // if empty -> local
};
