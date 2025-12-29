#pragma once
#include <QObject>


namespace qtpyt {

class QPyScript : public QObject {
Q_OBJECT
public:
static std::tuple<bool, QString> runScriptFileGlobal(const QString& script_path, QObject* root_obj);
static std::tuple<bool, QString> runScriptGlobal(const QString& script, QObject* root_obj);
};

} // namespace qtpyt

