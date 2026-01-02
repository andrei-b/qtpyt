#include "timepanel.h"

#include <QHBoxLayout>
#include <QTimeZone>

TimePanel::TimePanel(QWidget* parent)
    : QWidget(parent)
{
    m_label = new QLabel(this);
    m_label->setText("00:00:00");
    m_label->setAlignment(Qt::AlignCenter);

    // sensible default look; can be overridden via controller
    QFont f = m_label->font();
    f.setPointSize(28);
    f.setBold(true);
    m_label->setFont(f);

    // use palette-based colors by default
    setAutoFillBackground(true);

    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(12, 10, 12, 10);
    lay->addWidget(m_label, 1);

    m_timer.setInterval(200); // smooth second rollover; still light
    connect(&m_timer, &QTimer::timeout, this, &TimePanel::updateNow);
    m_timer.start();

    updateNow();
}

void TimePanel::setTimeFormat(const QString& fmt)
{
    m_format = fmt.trimmed().isEmpty() ? QString("HH:mm:ss") : fmt.trimmed();
    updateNow();
}

void TimePanel::setTimeZoneId(const QByteArray& tz)
{
    m_tzId = tz;
    updateNow();
}

void TimePanel::updateNow()
{
    QDateTime now = QDateTime::currentDateTime();

    if (!m_tzId.isEmpty()) {
        const QTimeZone tz(m_tzId);
        if (tz.isValid())
            now = now.toTimeZone(tz);
    }

    m_label->setText(now.toString(m_format));
}
