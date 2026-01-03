#pragma once
#include "../include/qtpyt/qpysharedarray.h"

#include <QObject>
#include <QString>
#include <QPoint>
#include <QVector3D>
#include <QVariant>
#include <QList>
#include <QMap>


class TestObject_2 : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool success READ success WRITE setSuccessProperty NOTIFY successPropertyChanged)

public:
    explicit TestObject_2(QObject* parent = nullptr) : QObject(parent) {
        qtpyt::registerSharedArray<double>("QPySharedArray<double>");
        qtpyt::registerQPairType<QString, int>("QPair<QString, int>");
    }

    bool success() const { return m_success; }
    void setSuccessProperty(bool b) {
        if (m_success == b) return;
        m_success = b;
        emit successPropertyChanged(m_success);
    }

    void emitInt(int v) { emit passInt(v); }
    void emitString(const QString& s) { emit passString(s); }
    void emitPoint(const QPoint& p) { emit passPoint(p); }
    void emitVector3D(const QVector3D& v) { emit passVector3D(v); }
    void emitVariant(const QVariant& v) { emit passVariant(v); }
    void emitVariantList(const QVariantList& xs) { emit passVariantList(xs); }
    void emitIntList(const QList<int>& xs) { emit passIntList(xs); }
    void emitStringIntMap(const QMap<QString, int>& m) { emit passStringIntMap(m); }
    void emitStringAndInt(const QString& s, int i) { emit passStringAndInt(s, i); }
    void emitSharedArray(const qtpyt::QPySharedArray<double>& arr) { emit passSharedArray(arr); }
    void emitQPair(const QPair<QString, int>& p) { emit passQPair(p); }
    void emitQVariantMap(const QVariantMap& m) { emit passQVariantMap(m); }

    signals:
    void successPropertyChanged(bool value);

    void passInt(int value);
    void passString(const QString& value);
    void passPoint(const QPoint& value);
    void passVector3D(const QVector3D& value);
    void passVariant(const QVariant& value);
    void passVariantList(const QVariantList& values);
    void passIntList(const QList<int>& values);
    void passStringIntMap(const QMap<QString, int>& value);
    void passStringAndInt(const QString& s, int i);
    void passSharedArray(const qtpyt::QPySharedArray<double>& array);
    void passQPair(const QPair<QString, int>& p);
    void passQVariantMap(const QVariantMap& map);

private:
    bool m_success{false};
};
