#pragma once

#include <QObject>
#include <QString>

namespace qtpyt {

/**
 * @brief PyRunner provides a simple interface for executing Python scripts in Qt applications.
 * 
 * This class wraps the QPyScript functionality and provides an easy-to-use API
 * for embedding Python in Qt applications.
 */
class PyRunner : public QObject {
    Q_OBJECT
    
public:
    explicit PyRunner(QObject* parent = nullptr);
    ~PyRunner() override;
    
    /**
     * @brief Initialize the Python interpreter.
     * @return true if initialization was successful, false otherwise
     */
    bool initialize();
    
    /**
     * @brief Check if the Python interpreter has been initialized.
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const;
    
    /**
     * @brief Execute a Python script.
     * @param script The Python code to execute
     * @return true if execution was successful, false otherwise
     */
    bool executeScript(const QString& script);
    
    /**
     * @brief Get the last error message.
     * @return The last error message, or an empty string if no error occurred
     */
    QString getLastError() const;
    
    /**
     * @brief Set the root object that will be accessible from Python scripts.
     * @param rootObject The QObject that Python scripts can interact with
     */
    void setRootObject(QObject* rootObject);
    
signals:
    /**
     * @brief Emitted when the initialization status changes.
     * @param initialized true if initialized, false otherwise
     */
    void initializationChanged(bool initialized);
    
    /**
     * @brief Emitted when a script execution completes.
     * @param success true if successful, false if there was an error
     * @param message Result or error message
     */
    void scriptExecuted(bool success, const QString& message);
    
private:
    bool m_initialized;
    QString m_lastError;
    QObject* m_rootObject;
};

} // namespace qtpyt
