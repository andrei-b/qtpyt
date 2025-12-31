#pragma once
#define PYBIND11_NO_KEYWORDS
#include <pybind11/pybind11.h>

#include <qtpyt/qpyannotation.h>
#include <qtpyt/pycall.h>
#include <QRunnable>
#include <QVariant>
#include <Python.h>

namespace py = pybind11;

namespace qtpyt {
    enum class QPySourceType {
        Module,
        File,
        SourceString
};

    enum class QPyArgumentType {
        Argument,
        VarArgs,
        VarKwargs
    };

    enum class QPyValueType {
        NoneType,
        Int,
        Float,
        Str,
        Bool,
        List,
        Dict,
        Tuple,
        Set,
        Object,
        Callable,
        Any,
         };

    struct QPythonArgument {
        QPyArgumentType argType;
        QPyValueType valueType;
        QString name;
        bool hasDefault = false;
        QPyAnnotation annotation;
    };

    struct PyCallableInfo {
        bool has_varargs = false;
        bool has_varkw = false;
        std::unordered_map<std::string, py::handle> annotations;   // param -> annotation (py::handle)
        std::vector<QPythonArgument> arguments;
        QPyValueType returnType;
    };

    using QPyRegisteredType = std::variant<QMetaType::Type, QString, QMetaType>;

    class QPyModuleBase {
    public:
        QPyModuleBase(const QString& source, QPySourceType sourceType);
        QPyModuleBase(const QPyModuleBase& other) = delete;
        QPyModuleBase& operator=(const QPyModuleBase& other) = delete;
        QPyModuleBase(const QPyModuleBase&& other) = delete;
        QPyModuleBase& operator=(const QPyModuleBase&& other) = delete;
        virtual ~QPyModuleBase();


        [[nodiscard]] bool isValid() const;
        [[nodiscard]] std::optional<QVariant> call(const QString& function, const QPyRegisteredType& returnType, const QVariantList& args, const QVariantMap& kwargs = {});
        std::optional<py::object> makeCallable(const QString& function);
        void setCallableFunction(const QString& name);
        template<typename R, typename... Args>
        R call(Args&&... args) const {
            pybind11::object varArgs = pycall_internal__::build_args_tuple(std::forward<Args>(args)...);
            const auto result = call(varArgs, {});
            return result.cast<R>();
        }
        template<typename R, typename... Args>
        R operator ()(Args&&... args) const  {
            return call<R>(std::forward<Args>(args)...);
        }
        [[nodiscard]] pybind11::object call(const pybind11::tuple& args, const pybind11::dict& kwargs) const;
        template<typename Signature>
        struct _pycall_return;

        template<typename R, typename... Args>
        struct _pycall_return<R(Args...)> { using type = R; };

        template<typename Signature>
        std::function<Signature> makeFunction(const QString& name = {}) {
            auto resultCallable = callable;
            if (name.isEmpty() || name == m_callableFunction) {
                if (!callable) throw std::runtime_error("callable is null");
            } else {
                resultCallable = m_module.attr(name.toStdString().c_str());
                if (!resultCallable || !PyCallable_Check(resultCallable.ptr())) {
                    throw std::runtime_error("callable is null");
                }
            }
            using R = typename _pycall_return<Signature>::type;
            m_isValid = false;
            return [callable=std::move(resultCallable)]<typename... T0>(T0&&... args) -> R {
                if constexpr (std::is_same_v<R, void>) {
                    pycall_internal__::call_python<R>(callable, std::forward<T0>(args)...);
                    return;
                } else {
                    return pycall_internal__::call_python<R>(callable, std::forward<T0>(args)...);            }
            };
        }
        const pybind11::object& pythonCallable() const;
        pybind11::object&& takePythonCallable(const QString& functionName);
        PyCallableInfo inspectCallable() const;
        QString functionName() const;
        void addVariable(const QString& name, const QVariant& value);
        QVariant readVariable(const QString& name, const QPyRegisteredType& type) const;
        template<typename T>
        void addVariable(const QString& name, const T& value) {
            addVariable(name, QVariant::fromValue(value));
        }
        template<typename T>
        T readVariable(const QString& name) const {
            QVariant var = readVariable(name, QMetaType::fromType<T>());
            if (var.isValid()) {
                return var.value<T>();
            }
            throw std::runtime_error("QPyModuleBase::readVariable: variable " + name.toStdString() + " is invalid or of wrong type");
        }
    protected:
        py::object &getPyModule();
    private:
        void buildFromString(const QString &source);
        void buildFromFile(const QString &fileName);
        QString m_callableFunction;
        pybind11::object callable;
        pybind11::object m_module;
        bool m_isValid{false};
    };
} // namespace qtpyt
