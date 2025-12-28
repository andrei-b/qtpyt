#include "qtpyt/PyRunner.h"

namespace qtpyt {

PyRunner::PyRunner() 
    : m_initialized(false) {
}

PyRunner::~PyRunner() {
    // Cleanup resources if needed
}

bool PyRunner::initialize() {
    if (m_initialized) {
        m_lastError = "PyRunner already initialized";
        return false;
    }
    
    // For now, just set the initialized flag
    // Actual Python initialization would go here
    m_initialized = true;
    m_lastError.clear();
    return true;
}

bool PyRunner::executeScript(const std::string& script) {
    if (!m_initialized) {
        m_lastError = "PyRunner not initialized";
        return false;
    }
    
    if (script.empty()) {
        m_lastError = "Empty script provided";
        return false;
    }
    
    // Actual script execution would go here
    // For now, just return success
    m_lastError.clear();
    return true;
}

bool PyRunner::isInitialized() const {
    return m_initialized;
}

std::string PyRunner::getLastError() const {
    return m_lastError;
}

void PyRunner::setPythonPath(const std::vector<std::string>& paths) {
    m_pythonPaths = paths;
}

} // namespace qtpyt
