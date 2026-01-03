#include <pybind11/pybind11.h>
#include "qtpyt/qpymodulebase.h"
#include "internal/qpymoduleimpl.h"
#include "q_embed_meta_object_py.h"
#include "internal/annotations.h"

#include "internal/q_py_execute_event.h"
#include "internal/pycall.h"

namespace qtpyt {

    QPyModuleBase::QPyModuleBase(const QString &source, const QPySourceType sourceType)  {
        m_internal = std::make_shared<QPyModuleImpl>(source, sourceType);
    }

    QPyModuleBase::~QPyModuleBase() {
            m_isValid = false;
    }

    bool QPyModuleBase::isValid() const {
        return m_internal->isValid();
    }

    std::pair<std::optional<QVariant>, QString> QPyModuleBase::call(const QString &function,
                                                                    const QPyRegisteredType &returnType,
                                                                    const QVariantList &args, const QVariantMap &kwargs) {
        return m_internal->call(function, returnType, args, kwargs);
    }

    void QPyModuleBase::setCallableFunction(const QString &name) {
        m_internal->setCallableFunction(name);
    }

    PyCallableInfo QPyModuleBase::inspectCallable() const {
        return m_internal->inspectCallable();
    }

    QString QPyModuleBase::functionName() const {
        return m_internal->functionName();
    }

    void QPyModuleBase::addVariable(const QString &name, const QVariant &value) {
        m_internal->addVariable(name, value);
    }

    void QPyModuleBase::addFunction(const QString &name, QVariantFn &&function) const {
        m_internal->addFunction(name, std::move(function));
    }

    QVariant QPyModuleBase::readVariable(const QString &name, const QPyRegisteredType &type) const {
        return m_internal->readVariable(name, type);
    }

    void QPyModuleBase::addFunctionInternal(const QString &name,
        const std::function<QVariant(const QVariantList)> &invokeFromList) const {
        m_internal->addFunctionInternal(name, invokeFromList);
    }

    QPyModuleImpl * QPyModuleBase::getInternal() {
        return m_internal.get();
    }

    void QPyModuleBase::buildFromString(const QString &source) {
       m_internal->buildFromString(source);
    }

    void QPyModuleBase::buildFromFile(const QString &fileName) {
        m_internal->buildFromFile(fileName);
    }
} // namespace qtpyt
