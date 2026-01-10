#pragma once
#include <qtpyt/qpysequencereference.h>
#include <QObject>
#include <QString>
#include <QPoint>
#include <QVector3D>
#include <QVariant>
#include <QMap>

#include "qtpyt/qpysharedbytearray.h"


class TestObject_2 : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool success READ success WRITE setSuccessProperty NOTIFY successPropertyChanged)
    Q_PROPERTY(int intResult READ intResult WRITE setIntResultProperty NOTIFY intResultPropertyChanged)
public:
    explicit TestObject_2(QObject* parent = nullptr) : QObject(parent) {
    }

    bool success() const { return m_success; }
    void setSuccessProperty(bool b) {
        if (m_success == b) return;
        m_success = b;
        emit successPropertyChanged(m_success);
    }
    int intResult() const { return m_intResult; }
    void setIntResultProperty(int v) {
        if (m_intResult == v) return;
        m_intResult = v;
        emit intResultPropertyChanged(m_intResult);
    }

    void emitInt(int v) { emit passInt(v); }
    void emitString(const QString& s) { emit passString(s); }
    void emitPoint(const QPoint& p) { emit passPoint(p); }
    void emitVector3D(const QVector3D& v) { emit passVector3D(v); }
    void emitVariant(const QVariant& v) { emit passVariant(v); }
    void emitVariantList(const QVariantList& xs) { emit passVariantList(xs); }
    void emitStringIntMap(const QMap<QString, int>& m) { emit passStringIntMap(m); }
    void emitStringAndInt(const QString& s, int i) { emit passStringAndInt(s, i); }
    void emitQPair(const QPair<QString, int>& p) { emit passQPair(p); }
    void emitQVariantMap(const QVariantMap& m) { emit passQVariantMap(m); }
    void emitSequenceReference(const qtpyt::QPySequenceReference& seqRef) {
        emit passSequenceReference(seqRef);
    }
    void emitByteArray(const QByteArray &ba) {
        emit passByteArray(ba);
    }
    void emitSharedByteArray(const qtpyt::QPySharedByteArray &sba) {
        emit passSharedByteArray(sba);
    }

    signals:
    void successPropertyChanged(bool value);
    void intResultPropertyChanged(int value);

    void passInt(int value);
    void passString(const QString& value);
    void passPoint(const QPoint& value);
    void passVector3D(const QVector3D& value);
    void passVariant(const QVariant& value);
    void passVariantList(const QVariantList& values);
    void passStringIntMap(const QMap<QString, int>& value);
    void passStringAndInt(const QString& s, int i);
    void passQPair(const QPair<QString, int>& p);
    void passQVariantMap(const QVariantMap& map);
    void passSequenceReference(const qtpyt::QPySequenceReference& seqRef);
    void passByteArray(const QByteArray &ba);
    void passSharedByteArray(const qtpyt::QPySharedByteArray &sba);

private:
    bool m_success{false};
    int m_intResult{0};
};
