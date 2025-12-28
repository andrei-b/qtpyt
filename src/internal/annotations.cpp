
#include <qtpyt/qpyannotation.h>
#include "annotations.h"

namespace annotations_internal__ {
static QString safe_repr(const py::handle &o) {
    try {
        return QString::fromStdString(py::str(py::repr(o)));
    } catch (...) {
        return "<unrepr-able>";
    }
}

static bool is_empty_annotation(const py::handle &ann) {
    py::object inspect = py::module_::import("inspect");
    py::object empty = inspect.attr("_empty");
    return ann.is(empty);
}

static py::object try_get_origin(const py::handle &ann) {
    try {
        py::object typing = py::module_::import("typing");
        // typing.get_origin(x) returns origin or None
        return typing.attr("get_origin")(ann);
    } catch (...) {
        return py::none();
    }
}

static py::object try_get_args(const py::handle &ann) {
    try {
        py::object typing = py::module_::import("typing");
        // typing.get_args(x) returns tuple()
        return typing.attr("get_args")(ann);
    } catch (...) {
        return py::tuple();
    }
}

QPyAnnotation parse_annotation(const py::handle &annotation, int depth) {
    QPyAnnotation result;
    result.repr = safe_repr(annotation);

    // Avoid runaway recursion on pathological types
    if (depth > 8) {
        result.has_annotation = true;
        result.base = "Unknown";
        return result;
    }

    // Missing annotation?
    if (is_empty_annotation(annotation)) {
        result.has_annotation = false;
        result.base = "Empty";
        return result;
    }

    result.has_annotation = true;

    // annotation is None? (rare, but possible if user literally wrote x: None)
    if (annotation.is_none()) {
        result.base = "None";
        return result;
    }

    // If annotation is a *string* forward reference:  x: "MyType"
    if (py::isinstance<py::str>(annotation)) {
        result.base = "ForwardRef";
        result.args.push_back(QPyAnnotation{true, false, QString::fromStdString(py::cast<std::string>(annotation)), {}, result.repr});
        return result;
    }

    // "Any" can be typing.Any, and "object" is Python's universal base class.
    // Treat both as "any type".
    try {
        py::object builtins = py::module_::import("builtins");
        py::object typing = py::module_::import("typing");
        if (annotation.is(builtins.attr("object")) || annotation.is(typing.attr("Any"))) {
            result.base = "Any";
            result.is_any = true;
            return result;
        }
    } catch (...) {
        // ignore
    }

    // Handle typing generics via get_origin/get_args:
    // Optional[int] -> origin=typing.Union, args=(int, NoneType)
    // list[int] -> origin=list, args=(int,)
    py::object origin = try_get_origin(annotation);
    py::object args_o = try_get_args(annotation);

    if (!origin.is_none()) {
        // Determine base name from origin
        // origin might be builtins.list, dict, collections.abc.Callable, etc.
        try {
            if (py::hasattr(origin, "__name__")) {
                result.base = QString::fromStdString(py::cast<std::string>(origin.attr("__name__")));
            } else {
                result.base = safe_repr(origin);
            }
        } catch (...) {
            result.base = "Generic";
        }

        // Recurse into args (tuple)
        try {
            py::tuple t = args_o.cast<py::tuple>();
            for (py::handle a : t) {
                result.args.push_back(parse_annotation(a, depth + 1));
            }
        } catch (...) {
            // ignore malformed args
        }

        // Special-case Optional: Union[T, NoneType]
        // If base=="Union" and any arg is NoneType -> Optional
        if (result.base == "Union") {
            bool has_none = false;
            std::vector<QPyAnnotation> non_none;
            for (auto &a : result.args) {
                if (a.base == "None") has_none = true;
                else non_none.push_back(a);
            }
            if (has_none && non_none.size() == 1) {
                QPyAnnotation opt;
                opt.has_annotation = true;
                opt.base = "Optional";
                opt.args = { non_none[0] };
                opt.repr = result.repr;
                return opt;
            }
        }

        return result;
    }

    // Non-generic case: builtins / classes / typing special forms
    // If it's a Python type/class, use __name__
    try {
        if (py::isinstance(annotation, py::module_::import("builtins").attr("type"))) {
            result.base = QString::fromStdString(py::cast<std::string>(py::getattr(annotation, "__name__", py::str("type"))));
            return result;
        }
    } catch (...) {
        // ignore
    }

    // Fallback: best-effort name extraction
    try {
        result.base = QString::fromStdString(py::cast<std::string>(py::getattr(annotation, "__name__", py::str("Unknown"))));
    } catch (...) {
        result.base = "Unknown";
    }
    return result;
}
}// namespace annotations_internal__