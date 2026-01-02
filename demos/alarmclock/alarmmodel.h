#pragma once

#include <QAbstractListModel>
#include <QDate>
#include <QTimer>

struct AlarmItem {
    int hour = 7;
    int minute = 0;
    QString label = "Alarm";
    bool enabled = true;

    // To avoid retriggering multiple times in the same day
    QDate lastTriggeredDate;
    bool ringing = false;
};

class AlarmModel final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool anyRinging READ anyRinging NOTIFY anyRingingChanged)
    Q_PROPERTY(int ringingIndex READ ringingIndex NOTIFY anyRingingChanged)

public:
    enum Roles {
        HourRole = Qt::UserRole + 1,
        MinuteRole,
        TimeTextRole,
        LabelRole,
        EnabledRole,
        RingingRole
    };

    explicit AlarmModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void addAlarm(int hour, int minute, const QString& label);
    Q_INVOKABLE void removeAlarm(int idx);
    Q_INVOKABLE void toggleEnabled(int idx);
    Q_INVOKABLE void updateLabel(int idx, const QString& label);

    Q_INVOKABLE void stopRinging();
    Q_INVOKABLE void snoozeMinutes(int minutes = 5);

    Q_INVOKABLE void save();
    Q_INVOKABLE void load();

    bool anyRinging() const;
    int ringingIndex() const;

    signals:
        void anyRingingChanged();

private:
    void checkTick();
    void setRinging(int idx, bool on);
    void clearAllRinging();

private:
    QVector<AlarmItem> m_items;
    QTimer m_tick;
};
