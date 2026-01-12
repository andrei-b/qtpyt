#include <private/qmetaobject_p.h>
#include <qtpyt/methodindex.h>

namespace  qtpyt {
    int getSignalCount(const QMetaObject* m) {
        if (!m) {
            return 0;
        }
        const QMetaObjectPrivate *mop = QMetaObjectPrivate::get(m);
        if (!mop) {
            return 0;
        }
        return mop->signalCount;
    }
}