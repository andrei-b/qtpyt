#include "internal/qpymemoryviewinternal.h"
#include "conversions.h"
#include "../include/qtpyt/qpymemoryview.h"

namespace qtpyt {
    int registerMemoryViewType() {
        return QPyMemoryView::registerMemoryViewType();
    }

    int QPyMemoryView::registerMemoryViewType() {

        int id = qRegisterMetaType<QPyMemoryView>("QPyMemoryView");
        addMetatypeVoidPtrToPyObjectConverterFunc(static_cast<QMetaType::Type>(id), [](const void *v) {
            auto *ptr = static_cast<QPyMemoryView *>(const_cast<void*>(v));
            return ptr->m_data->memoryview();
        });

        addFromQVariantFunc(id, [](const QVariant &v) {
            auto mv = v.value<QPyMemoryView>();
            auto obj = mv.m_data->memoryview();
            return obj;
        });

        QString typeName = QMetaType::typeName(id);
        addFromPyObjectToQVariantFunc(typeName, [](const py::object &obj) -> QVariant {
            auto pyObjectToQPyMemoryView = [](const py::object &obj) {
            py::gil_scoped_acquire gil;
            py::memoryview mv = py::reinterpret_borrow<py::memoryview>(obj);
            QSharedPointer<QPyMemoryViewInternal> internal = QSharedPointer<QPyMemoryViewInternal>::create(mv);
            QPyMemoryView qmv;
            qmv.m_data = internal;
            return qmv;
        };
            QPyMemoryView mv = pyObjectToQPyMemoryView(obj);
            return QVariant::fromValue(mv);
        });
        return id;
        return id;
    }

    QPyMemoryView::QPyMemoryView(char *ptr, char fmt, std::size_t itemCount, bool readOnly) {
        m_data = QSharedPointer<QPyMemoryViewInternal>::create(ptr, fmt, itemCount, readOnly);
    }

    std::size_t QPyMemoryView::nbytes() const noexcept {
        return    m_data->nbytes();
    }

    std::size_t QPyMemoryView::size() const noexcept {
        return    m_data->size();
    }

    std::size_t QPyMemoryView::itemsize() const noexcept {
        return    m_data->itemsize();
    }

    char QPyMemoryView::format() {
        return    m_data->format();
    }

    std::uint8_t * QPyMemoryView::data_u8() {
        return    m_data->data_u8();
    }

    const std::uint8_t * QPyMemoryView::cdata_u8() const {
        return    m_data->cdata_u8();
    }

    void QPyMemoryView::fill_byte(std::uint8_t v) {
        m_data->fill_byte(v);
    }

    void QPyMemoryView::writeData(std::size_t offset, const void *src, std::size_t len) {
        if (offset > nbytes() || len > (nbytes() - offset))
            throw std::out_of_range("writeData out of range");
        std::memcpy(data_u8() + offset, src, len);
    }

    void QPyMemoryView::readData(std::size_t offset, const void *dst, std::size_t len) {
        if (offset > nbytes() || len > (nbytes() - offset))
            throw std::out_of_range("readData out of range");
        std::memcpy(const_cast<void*>(dst), cdata_u8() + offset, len);
    }
} // namespace qtpyt