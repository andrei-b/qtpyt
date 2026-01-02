#define PYBIND11_NO_KEYWORDS

#include "qtpyt/PyRunner.h"
#include "qtpyt/qpyscript.h"
#include <pybind11/embed.h>

namespace qtpyt {

PyRunner::PyRunner(QObject* parent)
    : QObject(parent)
    , m_initialized(false)
    , m_lastError()
    , m_rootObject(nullptr)
{
}

PyRunner::~PyRunner() = default;

bool PyRunner::initialize() {
    if (m_initialized) {
        m_lastError = "Python interpreter already initialized";
        return true;
    }
    
    try {
        // Initialize the Python interpreter if not already initialized
        static bool interpreterInitialized = false;
        if (!interpreterInitialized) {
            static pybind11::scoped_interpreter guard{};
            interpreterInitialized = true;
        }
        
        m_initialized = true;
        m_lastError.clear();
        emit initializationChanged(true);
        return true;
    } catch (const std::exception& e) {
        m_lastError = QString("Failed to initialize Python: %1").arg(e.what());
        m_initialized = false;
        emit initializationChanged(false);
        return false;
    }
}

bool PyRunner::isInitialized() const {
    return m_initialized;
}

bool PyRunner::executeScript(const QString& script) {
    if (!m_initialized) {
        m_lastError = "Python interpreter not initialized. Call initialize() first.";
        emit scriptExecuted(false, m_lastError);
        return false;
    }
    
    if (script.isEmpty()) {
        m_lastError = "Script is empty";
        emit scriptExecuted(false, m_lastError);
        return false;
    }
    
    auto [success, errorMsg] = QPyScript::runScriptGlobal(script, m_rootObject);
    
    if (success) {
        m_lastError.clear();
        emit scriptExecuted(true, "Script executed successfully");
    } else {
        m_lastError = errorMsg;
        emit scriptExecuted(false, errorMsg);
    }
    
    return success;
}

QString PyRunner::getLastError() const {
    return m_lastError;
}

void PyRunner::setRootObject(QObject* rootObject) {
    m_rootObject = rootObject;
}

} // namespace qtpyt
