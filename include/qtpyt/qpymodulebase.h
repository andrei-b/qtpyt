/// \file qpymodulebase.h
/// \brief Base class for interacting with a Python module from C\+\+ using Qt types.
/// \details
/// `QPyModuleBase` encapsulates a Python module created from one of several source kinds
/// (imported module name, file path, or source string). It provides:
/// \- discovery of a callable within the module
/// \- typed and untyped invocation helpers
/// \- conversion between `pybind11` objects and Qt `QVariant`/`QMetaType`
/// \- registration of C\+\+ functions/variables into the Python module
///
/// Threading / GIL:
/// Most operations that touch Python objects require the Python GIL to be held.
/// This header does not explicitly acquire/release the GIL; callers and/or the
/// implementation are expected to ensure correct GIL handling.
///
/// Error handling:
/// Many API functions throw `std::runtime_error` for misuse (e.g. non\-callable objects,
/// conversion failures, wrong argument count). Some APIs return `std::optional` to
/// represent failure without throwing.

#pragma once
#include <qtpyt/qpyannotation.h>
#include <QRunnable>
#include <QVariant>

namespace qtpyt {

    /// \brief Describes how a `QPyModuleBase` should interpret its source string.
    enum class QPySourceType {
        /// \brief Treat the source as a Python module name to import.
        Module,
        /// \brief Treat the source as a file path to execute/load.
        File,
        /// \brief Treat the source as literal Python source code.
        SourceString
    };

    /// \brief Describes the role of a Python function parameter during inspection.
    enum class QPyArgumentType {
        /// \brief Regular positional or keyword parameter.
        Argument,
        /// \brief `\*args` varargs parameter.
        VarArgs,
        /// \brief `\*\*kwargs` varkwargs parameter.
        VarKwargs
    };

    /// \brief Coarse classification of Python values used for inspection and marshaling.
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
        /// \brief A value that is accepted/returned without strict type expectations.
        Any,
    };

    /// \brief Information about a single Python callable argument.
    struct QPythonArgument {
        /// \brief Parameter kind (regular / varargs / varkw).
        QPyArgumentType argType;
        /// \brief Expected/annotated value type classification.
        QPyValueType valueType;
        /// \brief Parameter name as declared in Python.
        QString name;
        /// \brief Whether the parameter has a default value.
        bool hasDefault = false;
        /// \brief Captured type annotation information (if present).
        QPyAnnotation annotation;
    };

    /// \brief Introspection results for a Python callable.
    struct PyCallableInfo {
        /// \brief True if the callable accepts `\*args`.
        bool has_varargs = false;
        /// \brief True if the callable accepts `\*\*kwargs`.
        bool has_varkw = false;

        /// \brief Argument list in declaration order as best as can be determined.
        std::vector<QPythonArgument> arguments;

        /// \brief Classified return value type.
        QPyValueType returnType;
    };

    /// \brief A "registered type" used to guide conversion between Python and Qt.
    /// Any (almost) Qt metatype can be specified using this variant.
    /// \details
    /// The variant supports specifying a type as:
    /// \- `QMetaType::Type` (built\-in Qt metatype id)
    /// \- `QString` (type name)
    /// \- `QMetaType` (full metatype object)
    using QPyRegisteredType = std::variant<QMetaType::Type, QString, QMetaType>;

    /// \brief Callable wrapper which takes a `QVariantList` and returns a `QVariant`.
    /// \details Used for exposing C\+\+ functions into Python by converting Python args
    /// \-\> `QVariantList` and converting the return `QVariant` back to Python.
    using QVariantFn = std::function<QVariant(const QVariantList&)>;

    class QPyModuleImpl;
    /// \class QPyModuleBase
    /// \brief Core functionality for building a Python module and invoking functions.
    /// \details
    /// Instances own references to Python objects (`pybind11::object`) representing a module
    /// and optionally a selected callable within that module. The concrete module creation
    /// is controlled by `QPySourceType`.
    ///
    /// Copy/move:
    /// Copy and move operations are disabled due to Python object semantics and internal
    /// state that should not be duplicated.
    class QPyModuleBase {
    public:
        /// \brief Constructs a module wrapper from Python source.
        /// \param source Module name, file path, or Python source text.
        /// \param sourceType Interpretation of \p source.
        /// \throws std::runtime_error on module construction failure.
        QPyModuleBase(const QString &source, QPySourceType sourceType);


        /// \brief Virtual destructor.
        /// \details Releases owned Python object references.
        virtual ~QPyModuleBase();

        /// \brief Indicates whether the module wrapper is currently in a valid state.
        /// \return True if the underlying module/callable state is valid for use.
        [[nodiscard]] bool isValid() const;

        /// \brief Calls a Python function by name with explicit Qt arguments.
        /// \param function Name of the Python callable to invoke.
        /// \param returnType Conversion target used to convert the Python return value.
        /// \param args Positional arguments.
        /// \param kwargs Keyword arguments (name \-\> value).
        /// \return Converted return value on success; `std::nullopt` on conversion failure.
        /// \throws std::runtime_error if the function cannot be found or invoked.
        [[nodiscard]] std::pair<std::optional<QVariant>, QString> call(const QString &function,
                                                                 const QPyRegisteredType &returnType,
                                                                 const QVariantList &args,
                                                                       const QVariantMap &kwargs = {});

        /// \brief Selects which function name is considered the "current" callable.
        /// \details Affects `pythonCallable()`, `call(tuple, dict)`, and `makeFunction()`.
        /// \param name Function name within the module.
        /// \throws std::runtime_error if the named attribute is missing or not callable.
        void setCallableFunction(const QString &name);

        /// \brief Calls a named Python function with typed return value.
        /// \tparam R Desired C\+\+ return type (or `void`).
        /// \tparam Args Argument types forwarded to Python via internal conversion.
        /// \param function Name of the Python callable to invoke.
        /// \param args Arguments forwarded to Python.
        /// \return The converted return value if \p R is not `void`.
        /// \throws std::runtime_error on invocation failure or return conversion failure.
        template<typename R, typename... Args>
        R call(const QString &function, Args &&... args) {
            QVariantList varArgs = {std::forward<Args>(args)...};
            setCallableFunction(function);
            auto result = call(function, QMetaType::fromType<R>(), varArgs);
            if constexpr (std::is_same_v<R, void>) {
                (void) result;
                return;
            } else {
                if (!result.first.has_value()) {
                    throw std::runtime_error("QPyModuleBase::call: " + result.second.toStdString());
                }
                return result.first.value().template value<R>();
            }
        }

        /// \brief Function\-call operator forwarding to \c call().
        /// \tparam R Desired C\+\+ return type.
        /// \tparam Args Argument types.
        /// \param args Arguments forwarded to Python.
        /// \return Return value converted to \p R.
        template<typename R, typename... Args>
        R operator ()(Args &&... args) const {
            return call<R>(std::forward<Args>(args)...);
        }


        /// \brief Helper metafunction to extract return type from a function signature.
        template<typename Signature>
        struct _pycall_return;

        /// \brief Specialization for `R(Args...)` signatures.
        template<typename R, typename... Args>
        struct _pycall_return<R(Args...)> {
            using type = R;
        };

        /// \brief Creates a C\+\+ `std::function` wrapper for a Python callable.
        /// \details
        /// If \p name is empty or equals the currently selected callable name, the current
        /// callable is used; otherwise the module attribute \p name is looked up.
        ///
        /// The returned `std::function` performs conversion and calls into Python.
        /// \tparam Signature Requested C\+\+ function signature.
        /// \param name Function name in the Python module.
        /// \return A callable object that invokes the Python callable.
        /// \throws std::runtime_error if the Python attribute is missing or not callable.
        template<typename Signature>
        std::function<Signature> makeFunction(const QString &name) {
            using R = typename _pycall_return<Signature>::type;
            return [module = this, name]<typename... T0>(T0 &&... args) -> R {
                QVariantList varArgs = {QVariant::fromValue(std::forward<T0>(args))...};
                    module->setCallableFunction(name);
                auto result = module->call(name, QMetaType::fromType<R>(), varArgs);
                if constexpr (std::is_same_v<R, void>) {
                    (void) result;
                    return;
                } else {
                    if (!result.first.has_value()) {
                        throw std::runtime_error("QPyModuleBase::makeFunction: " + result.second.toStdString());
                    }
                    return result.first.value().template value<R>();
                }
            };
        }


        /// \brief Inspects the currently selected callable for argument/return metadata.
        /// \return A `PyCallableInfo` describing parameters and return type.
        /// \throws std::runtime_error if inspection fails or no callable is selected.
        PyCallableInfo inspectCallable() const;

        /// \brief Returns the currently selected callable function name.
        /// \return Function name set by `setCallableFunction()`.
        QString functionName() const;

        /// \brief Adds/overwrites a variable in the Python module namespace.
        /// \param name Variable name.
        /// \param value Value converted from `QVariant` to a Python object.
        /// \throws std::runtime_error on conversion failure.
        void addVariable(const QString &name, const QVariant &value);

        /// \brief Registers a C\+\+ callback as a Python function in the module.
        /// \param name Function name exposed to Python.
        /// \param function Callback receiving positional args as `QVariantList`.
        /// \throws std::runtime_error on conversion failures at call time.
        void addFunction(const QString& name, QVariantFn&& function) const;

        /// \brief Registers a typed C\+\+ function as a Python function in the module.
        /// \tparam R Return type.
        /// \tparam Args Argument types.
        /// \param name Function name exposed to Python.
        /// \param function C\+\+ callable to be invoked from Python.
        /// \details
        /// Python positional args are converted to `QVariant`, then to the requested `Args...`.
        /// The return value is converted back to a Python object.
        /// \throws std::runtime_error if called with the wrong argument count or conversion fails.
        template<typename R, typename... Args>
        void addFunction(const QString& name, std::function<R(Args...)>&& function) const {
            std::function<QVariant(const QVariantList)> invokeFromList = [function = std::move(function)](const QVariantList& argList) -> QVariant {
                if (argList.size() != int(sizeof...(Args))) {
                    throw std::runtime_error("QPyModuleBase::addFunction: wrong argument count");
                }

                auto invokeImpl = [&]<std::size_t... I>(std::index_sequence<I...>) -> QVariant {
                    if constexpr (std::is_same_v<R, void>) {
                        function(argList[int(I)].template value<std::decay_t<Args>>()...);
                        return {};
                    } else {
                        R r = function(argList[int(I)].template value<std::decay_t<Args>>()...);
                        return QVariant::fromValue(r);
                    }
                };

                return invokeImpl(std::index_sequence_for<Args...>{});
            };
            addFunctionInternal(name, invokeFromList);
        }

        /// \brief Reads a variable from the Python module namespace and converts it.
        /// \param name Variable name.
        /// \param type Expected type information used for conversion.
        /// \return Converted value as a `QVariant` (may be invalid if not found).
        /// \throws std::runtime_error on conversion failure.
        QVariant readVariable(const QString &name, const QPyRegisteredType &type) const;

        /// \brief Adds a C\+\+ value as a Python variable using `QVariant::fromValue`.
        /// \tparam T Value type.
        /// \param name Variable name.
        /// \param value Value to store.
        template<typename T>
        void addVariable(const QString &name, const T &value) {
            addVariable(name, QVariant::fromValue(value));
        }

        /// \brief Reads a Python variable and converts it to \p T.
        /// \tparam T Desired C\+\+ type.
        /// \param name Variable name.
        /// \return Converted value.
        /// \throws std::runtime_error if the variable is missing or not convertible.
        template<typename T>
        T readVariable(const QString &name) const {
            QVariant var = readVariable(name, QMetaType::fromType<T>());
            if (var.isValid()) {
                return var.value<T>();
            }
            throw std::runtime_error(
                "QPyModuleBase::readVariable: variable " + name.toStdString() + " is invalid or of wrong type");
        }

    protected:
        void addFunctionInternal(const QString &name,
            const std::function<QVariant(const QVariantList)>& invokeFromList) const;
        /// \brief Provides access to the internal module implementation.
        /// \return Pointer to the internal module implementation.

        QPyModuleImpl* getInternal();

    private:
        /// \brief Builds/initializes the module from literal Python source code.
        /// \param source Python code to execute.
        /// \throws std::runtime_error on compilation/execution failure.
        void buildFromString(const QString &source);

        /// \brief Builds/initializes the module from a Python file.
        /// \param fileName Path to the Python file.
        /// \throws std::runtime_error on file/load failure.
        void buildFromFile(const QString &fileName);

        /// \brief Name of the currently selected callable within the module.
        QString m_callableFunction;
 /// \brief Cached validity state.
        bool m_isValid{false};
        std::shared_ptr<QPyModuleImpl> m_internal;
    };

} // namespace qtpyt
