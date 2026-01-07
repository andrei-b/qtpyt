#pragma once
#include <QString>


namespace conversions_internal__ {
    // Normalizes C++/Qt type names:
    // - trims
    // - removes leading "const", "volatile", and "struct"/"class"/"enum"
    // - removes reference/pointer suffixes (&, &&, *)
    // - optionally strips namespaces ("A::B" -> "B") even inside templates
    // - canonicalizes spaces around "<", ">", ",", "&", "*"
    // Note: does not try to be a full C++ parser; intended for Qt metatype names.
    QString normalizeTypeName(QString s, bool stripNamespaces = true);
} // namespace conversions_internal__
