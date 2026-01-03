#pragma once
#include <pybind11/pybind11.h>
#include <qtpyt/qpymodulebase.h>
#include <QVariant>

namespace qtpyt {
    class QPyModuleImpl {
    public:
        QPyModuleImpl(const QString &source, QPySourceType sourceType);

        QPyModuleImpl(const QPyModuleBase &other) = delete;
        QPyModuleImpl &operator=(const QPyModuleBase &other) = delete;
        QPyModuleImpl(const QPyModuleBase &&other) = delete;
        QPyModuleImpl &operator=(const QPyModuleBase &&other) = delete;
        virtual ~QPyModuleImpl();
        [[nodiscard]] bool isValid() const;
        [[nodiscard]] std::pair<std::optional<QVariant>, QString> call(const QString &function,
                                                                       const QPyRegisteredType &returnType,
                                                                       const QVariantList &args,
                                                                       const QVariantMap &kwargs = {});
        void setCallableFunction(const QString &name);
        PyCallableInfo inspectCallable() const;
        QString functionName() const;
        void addVariable(const QString &name, const QVariant &value);
        void addFunction(const QString& name, QVariantFn&& function) const;
        QVariant readVariable(const QString &name, const QPyRegisteredType &type) const;
        void addFunctionInternal(const QString &name,const std::function<QVariant(const QVariantList)>& invokeFromList) const;
        void buildFromString(const QString &source);
        void buildFromFile(const QString &fileName);
    private:
        QString m_callableFunction;
        bool m_isValid{false};
        pybind11::object callable;
        pybind11::object m_module;
    };

} // namespace qtpyt
