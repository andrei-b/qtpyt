#pragma once
#include <qtpyt/qpysequencereference.h>
#include <QObject>
#include <QString>
#include <QPoint>
#include <QVector3D>
#include <QVariant>
#include <QMap>


class TestObject_2 : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool success READ success WRITE setSuccessProperty NOTIFY successPropertyChanged)

public:
    explicit TestObject_2(QObject* parent = nullptr) : QObject(parent) {
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
    void emitStringIntMap(const QMap<QString, int>& m) { emit passStringIntMap(m); }
    void emitStringAndInt(const QString& s, int i) { emit passStringAndInt(s, i); }
    void emitQPair(const QPair<QString, int>& p) { emit passQPair(p); }
    void emitQVariantMap(const QVariantMap& m) { emit passQVariantMap(m); }
    void emitSequenceReference(const qtpyt::QPySequenceReference& seqRef) {
        emit passSequenceReference(seqRef);
    }

    signals:
    void successPropertyChanged(bool value);

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

private:
    bool m_success{false};
};
