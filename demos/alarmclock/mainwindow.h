// demos/alarmclock/mainwindow.h
#pragma once

#include <QMainWindow>
#include <QDateTime>

class AlarmEngine;
class TimePanel;

class QDateTimeEdit;
class QLineEdit;
class QComboBox;
class QSpinBox;
class QListWidget;
class QPushButton;
class QLabel;
class QGroupBox;
class QTimer;

class MainWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onAdd();
    void onRemoveSelected();
    void onToggleSelected();
    void onTest();
    void onStop();

    void refreshList();
    void onAlarmFired(int id, QDateTime when, QString label);
    void onAlarmStopped();

private:
    AlarmEngine* m_engine = nullptr;

    TimePanel* m_timePanel = nullptr;

    QDateTimeEdit* m_when = nullptr;
    QLineEdit* m_label = nullptr;
    QPushButton* m_add = nullptr;
    QPushButton* m_remove = nullptr;
    QPushButton* m_toggle = nullptr;

    QLineEdit* m_melody = nullptr;
    QComboBox* m_effect = nullptr;
    QSpinBox* m_volume = nullptr;
    QPushButton* m_test = nullptr;
    QPushButton* m_stop = nullptr;

    QListWidget* m_list = nullptr;
    QLabel* m_status = nullptr;

    // Visual alarm indication
    QGroupBox* m_alarmBanner = nullptr;
    QLabel* m_alarmBannerLabel = nullptr;
    QPushButton* m_alarmBannerStop = nullptr;
    QTimer* m_flashTimer = nullptr;
    bool m_alarmActive = false;
};
