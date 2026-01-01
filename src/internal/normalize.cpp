#include "normalize.h"

#include <QString>
#include <QRegularExpression>


namespace conversions_internal__ {
    // Normalizes C++/Qt type names:
    // - trims
    // - removes leading "const", "volatile", and "struct"/"class"/"enum"
    // - removes reference/pointer suffixes (&, &&, *)
    // - optionally strips namespaces ("A::B" -> "B") even inside templates
    // - canonicalizes spaces around "<", ">", ",", "&", "*"
    // Note: does not try to be a full C++ parser; intended for Qt metatype names.
    QString normalizeTypeName(QString s, bool stripNamespaces) {
        s = s.trimmed();

        // Drop common leading keywords/qualifiers.
        auto dropLeadingWord = [&](const QString& w) {
            if (s.startsWith(w + QLatin1Char(' '))) s = s.mid(w.size() + 1).trimmed();
            else if (s == w) s.clear();
        };
        // Iterate to handle "const volatile T"
        for (;;) {
            const QString before = s;
            dropLeadingWord(QStringLiteral("const"));
            dropLeadingWord(QStringLiteral("volatile"));
            dropLeadingWord(QStringLiteral("struct"));
            dropLeadingWord(QStringLiteral("class"));
            dropLeadingWord(QStringLiteral("enum"));
            if (s == before) break;
        }

        // Remove trailing reference/pointer qualifiers (keep removing while present).
        for (;;) {
            const QString before = s;
            s = s.trimmed();
            while (s.endsWith(QStringLiteral("&&"))) s.chop(2), s = s.trimmed();
            while (s.endsWith(QChar('&'))) s.chop(1), s = s.trimmed();
            while (s.endsWith(QChar('*'))) s.chop(1), s = s.trimmed();
            if (s == before) break;
        }

        // Normalize whitespace globally (collapse multiple spaces).
        s.replace(QRegularExpression(QStringLiteral("\\s+")), QStringLiteral(" "));

        // Canonical spacing around template punctuation/punctuators.
        // Remove spaces around < and >
        s.replace(QRegularExpression(QStringLiteral("\\s*<\\s*")), QStringLiteral("<"));
        s.replace(QRegularExpression(QStringLiteral("\\s*>\\s*")), QStringLiteral(">"));
        // Normalize commas: no space before, single space after
        s.replace(QRegularExpression(QStringLiteral("\\s*,\\s*")), QStringLiteral(", "));
        // Normalize pointer/reference tokens inside composite names
        s.replace(QRegularExpression(QStringLiteral("\\s*\\*\\s*")), QStringLiteral("*"));
        s.replace(QRegularExpression(QStringLiteral("\\s*&\\s*")), QStringLiteral("&"));

        if (!stripNamespaces) {
            return s.trimmed();
        }

        // Strip namespaces: replace each identifier chain like A::B::C with just C.
        // Applies both outside and inside templates.
        // This is a heuristic: it does not handle all C++ edge cases.
        s.replace(QRegularExpression(QStringLiteral("\\b(?:[A-Za-z_][A-Za-z0-9_]*::)+([A-Za-z_][A-Za-z0-9_]*)\\b")),
                  QStringLiteral("\\1"));

        return s.trimmed();
    }
}