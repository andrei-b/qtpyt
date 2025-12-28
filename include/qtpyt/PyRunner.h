#ifndef QTPYT_PYRUNNER_H
#define QTPYT_PYRUNNER_H

#include <string>
#include <vector>

namespace qtpyt {

/**
 * @brief PyRunner class for managing Python execution in Qt applications
 * 
 * This class provides a simple interface for embedding Python into C++ Qt applications
 * with support for multi-threaded execution.
 */
class PyRunner {
public:
    /**
     * @brief Construct a new PyRunner object
     */
    PyRunner();

    /**
     * @brief Destroy the PyRunner object
     */
    ~PyRunner();

    /**
     * @brief Initialize the Python interpreter
     * @return true if initialization was successful, false otherwise
     */
    bool initialize();

    /**
     * @brief Execute a Python script
     * @param script The Python script to execute
     * @return true if execution was successful, false otherwise
     */
    bool executeScript(const std::string& script);

    /**
     * @brief Check if the Python interpreter is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const;

    /**
     * @brief Get the last error message
     * @return The last error message as a string
     */
    std::string getLastError() const;

    /**
     * @brief Set the Python path
     * @param paths Vector of paths to add to Python's sys.path
     */
    void setPythonPath(const std::vector<std::string>& paths);

private:
    bool m_initialized;
    std::string m_lastError;
    std::vector<std::string> m_pythonPaths;
};

} // namespace qtpyt

#endif // QTPYT_PYRUNNER_H
