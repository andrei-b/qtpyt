// demos/alarmclock/mainwindow.cpp

#include "mainwindow.h"

#include "alarmengine.h"
#include "timepanel.h"

#include <QApplication>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include <QVariantMap>

static QVariantMap toMap(const QVariant& v)
{
    if (v.metaType().id() == QMetaType::QVariantMap)
        return v.toMap();
    if (v.metaType().id() == QMetaType::QVariantHash) {
        const auto h = v.toHash();
        QVariantMap m;
        for (auto it = h.begin(); it != h.end(); ++it)
            m.insert(it.key(), it.value());
        return m;
    }
    return {};
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    m_engine = new AlarmEngine(this);

    auto* central = new QWidget(this);
    auto* root = new QVBoxLayout(central);

    m_alarmBanner = new QGroupBox("Ringing", central);
    m_alarmBanner->setVisible(false);

    auto* bannerLayout = new QHBoxLayout(m_alarmBanner);
    m_alarmBannerLabel = new QLabel("Alarm ringing!", m_alarmBanner);

    QFont bf = m_alarmBannerLabel->font();
    bf.setBold(true);
    bf.setPointSize(bf.pointSize() + 2);
    m_alarmBannerLabel->setFont(bf);

    m_alarmBannerStop = new QPushButton("Stop", m_alarmBanner);
    bannerLayout->addWidget(m_alarmBannerLabel, 1);
    bannerLayout->addWidget(m_alarmBannerStop);

    root->addWidget(m_alarmBanner);

    m_flashTimer = new QTimer(this);
    m_flashTimer->setInterval(400);
    connect(m_flashTimer, &QTimer::timeout, this, [this] {
        if (m_alarmActive) QApplication::alert(this, 0);
    });

    connect(m_alarmBannerStop, &QPushButton::clicked, this, [this] {
        if (m_engine) m_engine->stopAlarm();
    });

    auto* schedBox = new QGroupBox("Schedule", central);
    auto* sched = new QHBoxLayout(schedBox);

    m_when = new QDateTimeEdit(QDateTime::currentDateTime().addSecs(60), schedBox);
    m_when->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    m_when->setCalendarPopup(true);

    m_timePanel = new TimePanel(central);
    root->addWidget(m_timePanel);

    m_label = new QLineEdit(schedBox);
    m_label->setPlaceholderText("Label");

    m_add = new QPushButton("Add", schedBox);
    m_remove = new QPushButton("Remove", schedBox);
    m_toggle = new QPushButton("Enable/Disable", schedBox);

    sched->addWidget(new QLabel("When:", schedBox));
    sched->addWidget(m_when, 1);
    sched->addWidget(new QLabel("Label:", schedBox));
    sched->addWidget(m_label, 1);
    sched->addWidget(m_add);
    sched->addWidget(m_remove);
    sched->addWidget(m_toggle);

    root->addWidget(schedBox);

    auto* cfgBox = new QGroupBox("Alarms", central);
    auto* cfg = new QHBoxLayout(cfgBox);

    m_melody = new QLineEdit(cfgBox);
    m_melody->setPlaceholderText("Melody path or URL (e.g. /home/me/alarm.mp3 or file:///...)");

    m_effect = new QComboBox(cfgBox);
    m_effect->addItem("None", int(AlarmEngine::Effect::None));
    m_effect->addItem("Beep", int(AlarmEngine::Effect::Beep));
    m_effect->addItem("FadeIn", int(AlarmEngine::Effect::FadeIn));
    m_effect->addItem("Pulse", int(AlarmEngine::Effect::Pulse));
    m_effect->setCurrentIndex(m_effect->findData(int(AlarmEngine::Effect::FadeIn)));

    m_volume = new QSpinBox(cfgBox);
    m_volume->setRange(0, 100);
    m_volume->setValue(80);

    m_test = new QPushButton("Test (3s)", cfgBox);
    m_stop = new QPushButton("Stop", cfgBox);

    cfg->addWidget(new QLabel("Melody:", cfgBox));
    cfg->addWidget(m_melody, 2);
    cfg->addWidget(new QLabel("Effect:", cfgBox));
    cfg->addWidget(m_effect);
    cfg->addWidget(new QLabel("Vol:", cfgBox));
    cfg->addWidget(m_volume);
    cfg->addWidget(m_test);
    cfg->addWidget(m_stop);

    root->addWidget(cfgBox);

    m_list = new QListWidget(central);
    root->addWidget(m_list, 1);

    m_status = new QLabel("Ready.", central);
    root->addWidget(m_status);

    setCentralWidget(central);
    setWindowTitle("Alarm Clock (Widgets)");

    connect(m_add, &QPushButton::clicked, this, &MainWindow::onAdd);
    connect(m_remove, &QPushButton::clicked, this, &MainWindow::onRemoveSelected);
    connect(m_toggle, &QPushButton::clicked, this, &MainWindow::onToggleSelected);
    connect(m_test, &QPushButton::clicked, this, &MainWindow::onTest);
    connect(m_stop, &QPushButton::clicked, this, &MainWindow::onStop);

    connect(m_engine, &AlarmEngine::alarmsChanged, this, &MainWindow::refreshList);
    connect(m_engine, &AlarmEngine::alarmFired, this, &MainWindow::onAlarmFired);
    connect(m_engine, &AlarmEngine::alarmStopped, this, &MainWindow::onAlarmStopped);

    connect(m_melody, &QLineEdit::editingFinished, this, [this] {
        if (m_engine) m_engine->setMelody(m_melody->text());
    });
    connect(m_volume, qOverload<int>(&QSpinBox::valueChanged), this, [this](int v) {
        if (m_engine) m_engine->setVolume(v);
    });
    connect(m_effect, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int idx) {
        if (!m_engine) return;
        const auto e = AlarmEngine::Effect(m_effect->itemData(idx).toInt());
        m_engine->setEffect(e);
    });

    refreshList();
}

void MainWindow::onAdd()
{
    if (!m_engine) return;

    const QDateTime when = m_when ? m_when->dateTime() : QDateTime::currentDateTime();
    const QString label = m_label ? m_label->text().trimmed() : QString();

    m_engine->addAlarm(when, /*enabled=*/true, /*repeatDaily=*/false, label);

    if (m_label) m_label->clear();
}

void MainWindow::onRemoveSelected()
{
    if (!m_engine || !m_list) return;

    auto* item = m_list->currentItem();
    if (!item) return;

    const int id = item->data(Qt::UserRole).toInt();
    m_engine->removeAlarm(id);
}

void MainWindow::onToggleSelected()
{
    if (!m_engine || !m_list) return;

    auto* item = m_list->currentItem();
    if (!item) return;

    const int id = item->data(Qt::UserRole).toInt();
    const bool enabled = item->data(Qt::UserRole + 1).toBool();

    m_engine->setAlarmEnabled(id, !enabled);
}

void MainWindow::onTest()
{
    if (!m_engine) return;
    m_engine->testAlarm(3000);
}

void MainWindow::onStop()
{
    if (!m_engine) return;
    m_engine->stopAlarm();
}

void MainWindow::refreshList()
{
    if (!m_list || !m_engine) return;

    m_list->clear();

    const auto alarms = m_engine->alarms(); // likely QVariantList
    for (const auto& v : alarms) {
        const QVariantMap m = toMap(v);
        if (m.isEmpty())
            continue;

        const int id = m.value("id").toInt();
        const bool enabled = m.value("enabled", true).toBool();
        const QDateTime when = m.value("when").toDateTime();
        const QString label = m.value("label").toString().trimmed();

        QString text = when.isValid() ? when.toString("yyyy-MM-dd HH:mm:ss") : QString("Alarm #%1").arg(id);
        if (!label.isEmpty())
            text += QString(" \u2014 %1").arg(label);
        if (!enabled)
            text += " (disabled)";

        auto* item = new QListWidgetItem(text, m_list);
        item->setData(Qt::UserRole, id);
        item->setData(Qt::UserRole + 1, enabled);
    }
}

void MainWindow::onAlarmFired(int id, QDateTime when, QString label)
{
    m_alarmActive = true;

    QString banner = QString("ALARM #%1  %2").arg(id).arg(when.toString("HH:mm:ss"));
    if (!label.trimmed().isEmpty())
        banner += QString(" \u2014 %1").arg(label.trimmed());

    if (m_alarmBannerLabel) m_alarmBannerLabel->setText(banner);
    if (m_alarmBanner) m_alarmBanner->setVisible(true);

    raise();
    activateWindow();

    if (m_flashTimer && !m_flashTimer->isActive())
        m_flashTimer->start();

    if (m_status)
        m_status->setText(QString("ALARM FIRED #%1 (%2) %3")
                              .arg(id)
                              .arg(when.toString("HH:mm:ss"))
                              .arg(label));
}

void MainWindow::onAlarmStopped()
{
    m_alarmActive = false;

    if (m_alarmBanner) m_alarmBanner->setVisible(false);
    if (m_flashTimer && m_flashTimer->isActive())
        m_flashTimer->stop();

    if (m_status) m_status->setText("Alarm stopped.");
}
