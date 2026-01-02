import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: win
    width: 420
    height: 720
    visible: true
    title: "Alarm Clock"

    property int newHour: 7
    property int newMinute: 0

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // Big clock
        Rectangle {
            Layout.fillWidth: true
            height: 110
            radius: 16
            border.width: 1
            color: "transparent"

            Text {
                anchors.centerIn: parent
                font.pixelSize: 44
                text: Qt.formatTime(new Date(), "hh:mm:ss")
            }

            Timer {
                interval: 250
                repeat: true
                running: true
                onTriggered: parent.children[0].text = Qt.formatTime(new Date(), "hh:mm:ss")
            }
        }

        // Add alarm card
        GroupBox {
            Layout.fillWidth: true
            title: "Add Alarm"

            ColumnLayout {
                anchors.fill: parent
                spacing: 10

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    SpinBox {
                        id: hourBox
                        from: 0; to: 23
                        value: win.newHour
                        editable: true
                        onValueChanged: win.newHour = value
                        Layout.preferredWidth: 90
                    }
                    Label { text: ":"; font.pixelSize: 18; Layout.alignment: Qt.AlignVCenter }

                    SpinBox {
                        id: minBox
                        from: 0; to: 59
                        value: win.newMinute
                        editable: true
                        onValueChanged: win.newMinute = value
                        Layout.preferredWidth: 90
                    }

                    TextField {
                        id: labelField
                        placeholderText: "Label (optional)"
                        Layout.fillWidth: true
                    }
                }

                Button {
                    text: "Add"
                    Layout.fillWidth: true
                    onClicked: {
                        alarmModel.addAlarm(win.newHour, win.newMinute, labelField.text)
                        labelField.text = ""
                    }
                }
            }
        }

        // Ringing banner
        Rectangle {
            Layout.fillWidth: true
            visible: alarmModel.anyRinging
            radius: 16
            height: visible ? 120 : 0
            color: "#ffefc2"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                Text {
                    font.pixelSize: 18
                    font.bold: true
                    text: {
                        if (alarmModel.ringingIndex < 0) return "Ringing!"
                        let a = alarmModel.get(alarmModel.ringingIndex) // may not exist; fallback below
                        return "Alarm ringing!"
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Button {
                        text: "Snooze 5 min"
                        Layout.fillWidth: true
                        onClicked: alarmModel.snoozeMinutes(5)
                    }
                    Button {
                        text: "Stop"
                        Layout.fillWidth: true
                        onClicked: alarmModel.stopRinging()
                    }
                }
            }
        }

        // Alarms list
        GroupBox {
            Layout.fillWidth: true
            Layout.fillHeight: true
            title: "Alarms"

            ListView {
                id: list
                anchors.fill: parent
                clip: true
                model: alarmModel
                spacing: 10

                delegate: Rectangle {
                    width: ListView.view.width
                    height: 80
                    radius: 16
                    border.width: 1
                    color: ringing ? "#ffe0e0" : "transparent"

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 12

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            Text {
                                font.pixelSize: 24
                                text: timeText
                            }
                            TextField {
                                text: label
                                placeholderText: "Label"
                                Layout.fillWidth: true
                                onEditingFinished: alarmModel.updateLabel(index, text)
                            }
                        }

                        Switch {
                            checked: enabled
                            onToggled: alarmModel.toggleEnabled(index)
                            Layout.alignment: Qt.AlignVCenter
                        }

                        Button {
                            text: "🗑"
                            onClicked: alarmModel.removeAlarm(index)
                            Layout.alignment: Qt.AlignVCenter
                        }
                    }
                }
            }
        }
    }
}
