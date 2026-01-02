#include "alarmengine.h"

#include <QVariantMap>
#include <QFileInfo>
#include <QtMath>

AlarmEngine::AlarmEngine(QObject* parent)
    : QObject(parent)
{
    m_audio = new QAudioOutput(this);
    m_audio->setVolume(qBound(0, m_volume, 100) / 100.0);

    m_player = new QMediaPlayer(this);
    m_player->setAudioOutput(m_audio);

    // tick every 250ms (simple + reliable). You can optimize to “sleep until next alarm”.
    m_tick.setInterval(250);
    connect(&m_tick, &QTimer::timeout, this, &AlarmEngine::onTick);
    m_tick.start();

    // effect animation
    m_effectTick.setInterval(50);
    connect(&m_effectTick, &QTimer::timeout, this, &AlarmEngine::onEffectTick);
}

int AlarmEngine::addAlarm(const QDateTime& when, bool enabled, bool repeatDaily, const QString& label)
{
    Alarm a;
    a.id = m_nextId++;
    a.when = when;
    a.enabled = enabled;
    a.repeatDaily = repeatDaily;
    a.label = label;

    m_alarms.push_back(a);
    emit alarmsChanged();
    return a.id;
}

bool AlarmEngine::removeAlarm(int id)
{
    for (int i = 0; i < m_alarms.size(); ++i) {
        if (m_alarms[i].id == id) {
            if (m_ringing && m_ringingAlarmId == id)
                stopAlarm();
            m_alarms.removeAt(i);
            emit alarmsChanged();
            return true;
        }
    }
    return false;
}

bool AlarmEngine::setAlarmEnabled(int id, bool enabled)
{
    for (auto& a : m_alarms) {
        if (a.id == id) {
            a.enabled = enabled;
            emit alarmsChanged();
            return true;
        }
    }
    return false;
}

bool AlarmEngine::setAlarmTime(int id, const QDateTime& when)
{
    for (auto& a : m_alarms) {
        if (a.id == id) {
            a.when = when;
            emit alarmsChanged();
            return true;
        }
    }
    return false;
}

QVariantList AlarmEngine::alarms() const
{
    QVariantList out;
    out.reserve(m_alarms.size());
    for (const auto& a : m_alarms) {
        QVariantMap m;
        m["id"] = a.id;
        m["when"] = a.when;
        m["enabled"] = a.enabled;
        m["repeatDaily"] = a.repeatDaily;
        m["label"] = a.label;
        out.push_back(m);
    }
    return out;
}

void AlarmEngine::setMelody(const QString& pathOrUrl)
{
    const QString trimmed = pathOrUrl.trimmed();
    if (trimmed == m_melody) return;
    m_melody = trimmed;

    if (!m_melody.isEmpty()) {
        m_player->setSource(toUrl(m_melody));
    } else {
        m_player->setSource(QUrl());
    }
    emit melodyChanged();
}

void AlarmEngine::setEffect(Effect e)
{
    if (m_effect == e) return;
    m_effect = e;
    emit effectChanged();
}

void AlarmEngine::setVolume(int v)
{
    v = qBound(0, v, 100);
    if (m_volume == v) return;
    m_volume = v;
    m_audio->setVolume(m_volume / 100.0);
    emit volumeChanged();
}

void AlarmEngine::testAlarm(int seconds)
{
    // Fire a synthetic alarm immediately.
    Alarm a;
    a.id = 0;
    a.when = QDateTime::currentDateTime();
    a.enabled = true;
    a.repeatDaily = false;
    a.label = "Test alarm";

    fireAlarm(a);

    QTimer::singleShot(qMax(1, seconds) * 1000, this, &AlarmEngine::stopAlarm);
}

void AlarmEngine::stopAlarm()
{
    if (!m_ringing) return;

    m_ringing = false;
    m_ringingAlarmId = 0;

    applyEffectStop();
    m_player->stop();

    emit alarmStopped();
}

void AlarmEngine::onTick()
{
    if (m_ringing) return;

    const auto now = QDateTime::currentDateTime();
    for (auto& a : m_alarms) {
        if (!a.enabled) continue;

        // fire when now >= scheduled time (with small tolerance)
        if (a.when.isValid() && a.when <= now) {
            fireAlarm(a);

            // handle repeatDaily: push to next day at same time
            if (a.repeatDaily) {
                a.when = a.when.addDays(1);
            } else {
                a.enabled = false;
            }
            emit alarmsChanged();
            break;
        }
    }
}

void AlarmEngine::fireAlarm(const Alarm& a)
{
    m_ringing = true;
    m_ringingAlarmId = a.id;

    // Ensure player source is set
    if (!m_melody.isEmpty())
        m_player->setSource(toUrl(m_melody));

    applyEffectStart();
    startPlayback();

    emit alarmFired(a.id, a.when, a.label);
}

void AlarmEngine::startPlayback()
{
    // If no melody: fallback to “beep” effect (system beep-like behavior isn’t portable).
    // We’ll still start the effect timer; the "Beep" effect just pulses volume.
    if (m_player->source().isEmpty()) {
        // nothing to play, but effects can still run (pulse volume won't matter much)
        return;
    }

    // loop-ish: when finished, restart if still ringing
    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus st) {
        if (!m_ringing) return;
        if (st == QMediaPlayer::EndOfMedia) {
            m_player->setPosition(0);
            m_player->play();
        }
    });

    m_player->setPosition(0);
    m_player->play();
}

void AlarmEngine::applyEffectStart()
{
    m_effectStep = 0;

    switch (m_effect) {
    case Effect::None:
        m_audio->setVolume(m_volume / 100.0);
        m_effectTick.stop();
        break;

    case Effect::Beep:
        // Quick pulsing volume (works for melody playback too)
        m_effectTick.start();
        break;

    case Effect::FadeIn:
        // start quiet and ramp to target
        m_audio->setVolume(0.05);
        m_effectTick.start();
        break;

    case Effect::Pulse:
        m_effectTick.start();
        break;
    }
}

void AlarmEngine::applyEffectStop()
{
    m_effectTick.stop();
    m_effectStep = 0;
    m_audio->setVolume(m_volume / 100.0);
}

void AlarmEngine::onEffectTick()
{
    if (!m_ringing) {
        m_effectTick.stop();
        return;
    }

    const double target = m_volume / 100.0;

    switch (m_effect) {
    case Effect::None:
        m_audio->setVolume(target);
        m_effectTick.stop();
        return;

    case Effect::FadeIn: {
        // ramp over ~3 seconds (50ms * 60 steps)
        const int steps = 60;
        const double t = qBound(0.0, double(m_effectStep) / steps, 1.0);
        const double v = qMax(0.05, target * t);
        m_audio->setVolume(v);
        m_effectStep++;
        if (m_effectStep > steps) {
            m_audio->setVolume(target);
            m_effectTick.stop();
        }
        return;
    }

    case Effect::Beep: {
        // square-ish pulse 2 Hz
        // step at 50ms => 20 steps/sec
        const int periodSteps = 10; // 2 Hz => 0.5s => 10 steps
        const bool on = (m_effectStep % periodSteps) < (periodSteps / 2);
        m_audio->setVolume(on ? target : qMin(target, 0.10));
        m_effectStep++;
        return;
    }

    case Effect::Pulse: {
        // smooth sinus pulse between 30%..100%
        const double base = 0.30 * target;
        const double amp  = 0.70 * target;
        const double phase = (m_effectStep % 80) / 80.0; // ~4s cycle
        const double s = (qSin(phase * 2.0 * M_PI) + 1.0) * 0.5; // 0..1
        m_audio->setVolume(base + amp * s);
        m_effectStep++;
        return;
    }
    }
}

QUrl AlarmEngine::toUrl(const QString& pathOrUrl)
{
    // Accept:
    //  - "file:///..." URL
    //  - "/home/x/alarm.mp3"
    //  - "C:\...\alarm.wav" (Windows)
    //  - "https://..." (stream)
    const QUrl u(pathOrUrl);
    if (u.isValid() && !u.scheme().isEmpty())
        return u;

    QFileInfo fi(pathOrUrl);
    return QUrl::fromLocalFile(fi.absoluteFilePath());
}
