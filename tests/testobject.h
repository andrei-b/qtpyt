#pragma once
#include <qtpyt/qpysharedarray.h>
#include <QObject>
#include <QPoint>
#include <QSize>
#include <QByteArray>
#include <QString>
#include <QDebug>
#include <list>
#include <map>


class TestObj : public QObject {
    Q_OBJECT
    Q_PROPERTY(QPoint value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(int intProperty READ intProperty WRITE setIntProperty)

public:
    TestObj() = default;

    Q_INVOKABLE void setSize(int w, int h) {
        lastCalled = "int";
        sizeByInts = QSize(w, h);
    }

    Q_INVOKABLE void setSize(const QSize &s) {
        lastCalled = "qsize";
        sizeByQSize = s;
    }

    Q_INVOKABLE void setSize2(const QSize &s) {
        lastCalled = "qsize";
        sizeByQSize = s;
    }

    Q_INVOKABLE bool setTitle(const QString &t) {
        title = t;
        qDebug() << "SetTitle called" << t;
        return true;
    }

    Q_INVOKABLE QString getTitle() const {
        qDebug() << "GetTitle called";
        return title;
    }


    void emitPassPoint(const QPoint &p) {
        emit passPoint(p);
    }

    void emitArray(const qtpyt::QPySharedArray<float> &arr) {
        emit passArray(arr);
    }

    Q_INVOKABLE void setPoints(const QPoint &p1, const QPoint &p2) {
        emit methodCalled("setPoints called");
        qDebug() << "setPoints called" << p1 << p2;
    }

    Q_INVOKABLE QList<int> getInts() {
        return {6, 7, 8, 9, 0};
    }

    Q_INVOKABLE QList<int> mulVect(const QList<int> &a, int c) {
        QList<int> result;
        for (int v: a) {
            result.push_back(v * c);
        }
        return result;
    };
    Q_INVOKABLE std::map<int, QString> retMap() {
        return {{1, "Apple"}, {2, "Orange"}, {3, "Banana"}};
    }

    QPoint m_value;
    QPoint value() const { return m_value; }

    void setValue(const QPoint &v) {
        if (m_value != v) {
            m_value = v;
            emit valueChanged(m_value);
        }
    }

    int intProperty() const { return m_intProperty; }
    void setIntProperty(int v) { m_intProperty = v; }
    QString getLastCalled() {
        return lastCalled;
    }
    int m_intProperty = 0;
    QString lastCalled;
    QSize sizeByInts;
    QSize sizeByQSize;
    QString title;
signals:
    void valueChanged(const QPoint &newValue);

    void passBA(const QByteArray &data);

    void passPoint(const QPoint &p);

    void passArray(const qtpyt::QPySharedArray<float> &arr);

    void methodCalled(const QString& msg);
};
