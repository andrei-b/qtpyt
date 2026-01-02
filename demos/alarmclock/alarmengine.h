#pragma once

#include <QObject>
#include <QDateTime>
#include <QTimer>
#include <QVector>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>

class AlarmEngine final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString melody READ melody WRITE setMelody NOTIFY melodyChanged)
    Q_PROPERTY(Effect effect READ effect WRITE setEffect NOTIFY effectChanged)
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY volumeChanged) // 0..100

public:
    enum class Effect {
        None = 0,
        Beep,
        FadeIn,
        Pulse
    };
    Q_ENUM(Effect)

    struct Alarm {
        int id = 0;
        QDateTime when;
        bool enabled = true;
        bool repeatDaily = false; // minimal example
        QString label;
    };

    explicit AlarmEngine(QObject* parent = nullptr);
    ~AlarmEngine() override = default;

    // --- INVOKABLE API (good for bindings / meta-call / scripting) ---
    Q_INVOKABLE int  addAlarm(const QDateTime& when, bool enabled = true, bool repeatDaily = false, const QString& label = {});
    Q_INVOKABLE bool removeAlarm(int id);
    Q_INVOKABLE bool setAlarmEnabled(int id, bool enabled);
    Q_INVOKABLE bool setAlarmTime(int id, const QDateTime& when);
    Q_INVOKABLE QVariantList alarms() const; // list of QVariantMap

    Q_INVOKABLE void stopAlarm();
    Q_INVOKABLE void testAlarm(int seconds = 3);

    // Config
    Q_INVOKABLE void setMelody(const QString& pathOrUrl);
    QString melody() const { return m_melody; }

    Q_INVOKABLE void setEffect(Effect e);
    Effect effect() const { return m_effect; }

    Q_INVOKABLE void setVolume(int v); // 0..100
    int volume() const { return m_volume; }

signals:
    void alarmFired(int id, QDateTime when, QString label);
    void alarmStopped();

    void melodyChanged();
    void effectChanged();
    void volumeChanged();
    void alarmsChanged();

private slots:
    void onTick();
    void onEffectTick();

private:
    void rescheduleTick();
    int  findNextEnabledAlarmIndex() const;
    void fireAlarm(const Alarm& a);
    void startPlayback();
    void applyEffectStart();
    void applyEffectStop();

    static QUrl toUrl(const QString& pathOrUrl);

private:
    QVector<Alarm> m_alarms;
    int m_nextId = 1;

    QTimer m_tick;        // checks next alarm boundary
    QTimer m_effectTick;  // effect animation timer

    QString m_melody;
    Effect  m_effect = Effect::FadeIn;
    int     m_volume = 80;

    bool m_ringing = false;
    int  m_ringingAlarmId = 0;

    // Effect state
    int m_effectStep = 0;

    QMediaPlayer* m_player = nullptr;
    QAudioOutput* m_audio  = nullptr;
};
