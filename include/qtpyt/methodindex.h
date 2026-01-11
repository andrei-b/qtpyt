#pragma once
#include <QMetaObject>



namespace qtpyt {
    int getSignalCount(const QMetaObject* m);

    template <typename SignalFunc>
    static std::optional<int> getMethodIndex(const QObject* sender, SignalFunc signal) {
        if (!signal) {
            qWarning("QObject::connect: invalid nullptr parameter");
            return std::nullopt;
        }

        int localIndex = -1;
        SignalFunc signalVar = signal;
        void* args[] = { &localIndex, reinterpret_cast<void*>(&signalVar) };

        const QMetaObject* originalMeta = sender->metaObject();
        const QMetaObject* foundMeta = nullptr;

        for (const QMetaObject* m = originalMeta; m && localIndex < 0; m = m->superClass()) {
            m->static_metacall(QMetaObject::IndexOfMethod, 0, args);
            if (localIndex >= 0 && localIndex < getSignalCount(m)) {
                foundMeta = m;
                break;
            }
        }

        if (!foundMeta) {
            qWarning("QObject::connect: signal not found in %s", originalMeta->className());
            return std::nullopt;
        }

        int absoluteMethodIndex = foundMeta->methodOffset() + localIndex;
        if (absoluteMethodIndex < 0 || absoluteMethodIndex >= originalMeta->methodCount()) {
            qWarning("QObject::connect: computed method index out of range for %s", foundMeta->className());
            return std::nullopt;
        }

        return absoluteMethodIndex;
    }
}