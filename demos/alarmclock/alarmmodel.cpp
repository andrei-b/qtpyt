#include "alarmmodel.h"

#include <QDateTime>
#include <QSettings>
#include <QtMath>

static QString pad2(int v) { return QString("%1").arg(v, 2, 10, QLatin1Char('0')); }

AlarmModel::AlarmModel(QObject* parent)
    : QAbstractListModel(parent)
{
    m_tick.setInterval(1000); // 1s tick
    m_tick.setTimerType(Qt::CoarseTimer);
    connect(&m_tick, &QTimer::timeout, this, &AlarmModel::checkTick);
    m_tick.start();
}

int AlarmModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return m_items.size();
}

QVariant AlarmModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return {};
    const int i = index.row();
    if (i < 0 || i >= m_items.size()) return {};

    const auto& a = m_items[i];
    switch (role) {
        case HourRole: return a.hour;
        case MinuteRole: return a.minute;
        case TimeTextRole: return pad2(a.hour) + ":" + pad2(a.minute);
        case LabelRole: return a.label;
        case EnabledRole: return a.enabled;
        case RingingRole: return a.ringing;
        default: return {};
    }
}

bool AlarmModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid()) return false;
    const int i = index.row();
    if (i < 0 || i >= m_items.size()) return false;

    auto& a = m_items[i];
    bool changed = false;

    switch (role) {
        case HourRole: {
            int v = value.toInt();
            v = qBound(0, v, 23);
            if (a.hour != v) { a.hour = v; changed = true; }
            break;
        }
        case MinuteRole: {
            int v = value.toInt();
            v = qBound(0, v, 59);
            if (a.minute != v) { a.minute = v; changed = true; }
            break;
        }
        case LabelRole: {
            const QString v = value.toString();
            if (a.label != v) { a.label = v; changed = true; }
            break;
        }
        case EnabledRole: {
            bool v = value.toBool();
            if (a.enabled != v) { a.enabled = v; changed = true; }
            break;
        }
        default:
            break;
    }

    if (changed) {
        emit dataChanged(index, index, {role, TimeTextRole});
        save();
    }
    return changed;
}

Qt::ItemFlags AlarmModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

QHash<int, QByteArray> AlarmModel::roleNames() const
{
    return {
        {HourRole, "hour"},
        {MinuteRole, "minute"},
        {TimeTextRole, "timeText"},
        {LabelRole, "label"},
        {EnabledRole, "enabled"},
        {RingingRole, "ringing"},
    };
}

void AlarmModel::addAlarm(int hour, int minute, const QString& label)
{
    AlarmItem a;
    a.hour = qBound(0, hour, 23);
    a.minute = qBound(0, minute, 59);
    a.label = label.trimmed().isEmpty() ? "Alarm" : label.trimmed();
    a.enabled = true;

    beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
    m_items.push_back(a);
    endInsertRows();

    emit anyRingingChanged();
    save();
}

void AlarmModel::removeAlarm(int idx)
{
    if (idx < 0 || idx >= m_items.size()) return;
    beginRemoveRows(QModelIndex(), idx, idx);
    m_items.removeAt(idx);
    endRemoveRows();

    emit anyRingingChanged();
    save();
}

void AlarmModel::toggleEnabled(int idx)
{
    if (idx < 0 || idx >= m_items.size()) return;
    auto& a = m_items[idx];
    a.enabled = !a.enabled;
    const QModelIndex mi = index(idx);
    emit dataChanged(mi, mi, {EnabledRole});
    save();
}

void AlarmModel::updateLabel(int idx, const QString& label)
{
    if (idx < 0 || idx >= m_items.size()) return;
    auto& a = m_items[idx];
    const QString v = label.trimmed().isEmpty() ? "Alarm" : label.trimmed();
    if (a.label == v) return;
    a.label = v;
    const QModelIndex mi = index(idx);
    emit dataChanged(mi, mi, {LabelRole});
    save();
}

bool AlarmModel::anyRinging() const
{
    for (const auto& a : m_items) if (a.ringing) return true;
    return false;
}

int AlarmModel::ringingIndex() const
{
    for (int i = 0; i < m_items.size(); ++i) if (m_items[i].ringing) return i;
    return -1;
}

void AlarmModel::setRinging(int idx, bool on)
{
    if (idx < 0 || idx >= m_items.size()) return;
    auto& a = m_items[idx];
    if (a.ringing == on) return;

    a.ringing = on;
    emit dataChanged(index(idx), index(idx), {RingingRole});
    emit anyRingingChanged();
}

void AlarmModel::clearAllRinging()
{
    bool any = false;
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].ringing) {
            m_items[i].ringing = false;
            emit dataChanged(index(i), index(i), {RingingRole});
            any = true;
        }
    }
    if (any) emit anyRingingChanged();
}

void AlarmModel::stopRinging()
{
    clearAllRinging();
}

void AlarmModel::snoozeMinutes(int minutes)
{
    const int idx = ringingIndex();
    if (idx < 0) return;

    // Stop current ring
    setRinging(idx, false);

    // Create a one-off “snooze alarm” by adjusting time to now + minutes
    const QDateTime now = QDateTime::currentDateTime();
    const QDateTime target = now.addSecs(qMax(1, minutes) * 60);

    auto& a = m_items[idx];
    a.hour = target.time().hour();
    a.minute = target.time().minute();
    a.lastTriggeredDate = QDate(); // allow trigger today again if needed

    emit dataChanged(index(idx), index(idx), {HourRole, MinuteRole, TimeTextRole});
    emit anyRingingChanged();
    save();
}

void AlarmModel::checkTick()
{
    const QDateTime now = QDateTime::currentDateTime();
    const QTime t = now.time();
    const QDate d = now.date();

    if (anyRinging()) {
        return; // QML/QtMultimedia handles looping sound
    }

    // Trigger alarms when current HH:MM matches; avoid re-trigger within same day.
    for (int i = 0; i < m_items.size(); ++i) {
        auto& a = m_items[i];
        if (!a.enabled) continue;
        if (a.hour == t.hour() && a.minute == t.minute()) {
            if (a.lastTriggeredDate == d) continue; // already fired today
            a.lastTriggeredDate = d;
            setRinging(i, true);
            save();
            break;
        }
    }
}

void AlarmModel::save()
{
    QSettings s;
    s.beginGroup("alarms");
    s.remove(""); // clear group
    s.setValue("count", m_items.size());

    for (int i = 0; i < m_items.size(); ++i) {
        const auto& a = m_items[i];
        s.beginGroup(QString("a%1").arg(i));
        s.setValue("hour", a.hour);
        s.setValue("minute", a.minute);
        s.setValue("label", a.label);
        s.setValue("enabled", a.enabled);
        s.setValue("lastTriggeredDate", a.lastTriggeredDate.isValid() ? a.lastTriggeredDate.toString(Qt::ISODate) : QString());
        s.endGroup();
    }

    s.endGroup();
}

void AlarmModel::load()
{
    QSettings s;
    s.beginGroup("alarms");
    const int count = s.value("count", 0).toInt();

    beginResetModel();
    m_items.clear();
    m_items.reserve(qMax(0, count));

    for (int i = 0; i < count; ++i) {
        s.beginGroup(QString("a%1").arg(i));
        AlarmItem a;
        a.hour = qBound(0, s.value("hour", 7).toInt(), 23);
        a.minute = qBound(0, s.value("minute", 0).toInt(), 59);
        a.label = s.value("label", "Alarm").toString();
        a.enabled = s.value("enabled", true).toBool();
        const QString last = s.value("lastTriggeredDate", "").toString();
        if (!last.isEmpty())
            a.lastTriggeredDate = QDate::fromString(last, Qt::ISODate);
        s.endGroup();
        m_items.push_back(a);
    }

    endResetModel();
    s.endGroup();

    emit anyRingingChanged();
}
