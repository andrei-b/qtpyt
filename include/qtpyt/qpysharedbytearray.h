#pragma once
#include <QByteArray>

namespace qtpyt {
    class QPySharedByteArray  {
    public:
        static QPySharedByteArray fromQString(const QString& s) {
            QPySharedByteArray ba;
            const QByteArray byteArray = s.toUtf8();
            ba.resize(static_cast<std::size_t>(byteArray.size()));
            if (ba.size() > 0) {
                std::memcpy(ba.data(), byteArray.constData(), ba.size());
            }
            return ba;
        }
        QString toQString() {
            if (this->isEmpty()) {
                return QString();
            }
            return QString::fromUtf8(this->data(), static_cast<int>(this->size()));
        }
        static QPySharedByteArray fromRawData(const char* data, std::size_t len) {
            QPySharedByteArray ba;
            ba.resize(len);
            if (len > 0) {
                std::memcpy(ba.data(), data, len);
            }
            return ba;
        }
        static QPySharedByteArray fromStdString(const std::string& s) {
            return fromRawData(s.data(), s.size());
        }
        static QPySharedByteArray fromStringView(const std::string_view& sv) {
            return fromRawData(sv.data(), sv.size());
        }
        static QPySharedByteArray fromQStringView(const QStringView& sv) {
            QPySharedByteArray ba;
            const QByteArray byteArray = sv.toUtf8();
            ba.resize(static_cast<std::size_t>(byteArray.size()));
            if (ba.size() > 0) {
                std::memcpy(ba.data(), byteArray.constData(), ba.size());
            }
            return ba;
        }
        void append(const char* data, std::size_t len) {
            const std::size_t oldSize = this->size();
            this->resize(oldSize + len);
            if (len > 0) {
                std::memcpy(this->data() + oldSize, data, len);
            }
        }
        QPySharedByteArray subString(std::size_t pos, std::size_t len) const {
            QPySharedByteArray sub;
            if (pos >= this->size()) {
                return sub;
            }
            const std::size_t availableLen = this->size() - pos;
            const std::size_t subLen = (len > availableLen) ? availableLen : len;
            sub.resize(subLen);
            if (subLen > 0) {
                std::memcpy(sub.data(), this->data() + pos, subLen);
            }
            return sub;
        }
        QPySharedByteArray() = default;
        explicit QPySharedByteArray(const QByteArray& ba)  {}
        void resize(std::size_t newSize) {
            if (!m_dataBlock) {
                m_dataBlock = std::shared_ptr<DataBlock>(new DataBlock(), [](DataBlock* block) {
                    delete[] block->m_data;
                    delete block;
                });
            }
            if (newSize != 0 && m_dataBlock->m_size == newSize) {
                return;
            }
            const char* oldData = m_dataBlock->m_data;
            const auto oldSize = m_dataBlock->m_size;
            m_dataBlock->m_data = new char[newSize];
            m_dataBlock->m_size = newSize;
            if (oldData) {
                const std::size_t copySize = (oldSize < newSize) ? oldSize : newSize;
                std::memcpy(m_dataBlock->m_data, oldData, copySize);
            }
            delete[] oldData;
        }
        char* data() {
            if (m_dataBlock) {
                return m_dataBlock->m_data;
            }
            return nullptr;
        }
        const char* data() const {
            if (m_dataBlock) {
                return m_dataBlock->m_data;
            }
            return nullptr;
        }
        std::size_t size() const {
            if (m_dataBlock) {
                return m_dataBlock->m_size;
            }
            return 0;
        }
        char& operator [](std::uint64_t idx) {
                return m_dataBlock->m_data[idx];
        }
        char at(std::uint64_t idx) const {
            if (m_dataBlock && idx >= 0 && static_cast<std::size_t>(idx) < m_dataBlock->m_size) {
                return m_dataBlock->m_data[idx];
            }
            throw std::out_of_range("QPySharedByteArray::at: index out of range");
        }
        char* begin() {
            if (m_dataBlock) {
                return m_dataBlock->m_data;
            }
            return nullptr;
        }
        char* end() {
            if (m_dataBlock) {
                return m_dataBlock->m_data + m_dataBlock->m_size;
            }
            return nullptr;
        }
        bool isEmpty() const {
            return size() == 0;
        }
        bool isReadOnly() const {
            if (m_dataBlock) {
                return m_dataBlock->m_readOnly;
            }
            return false;
        }
        void setReadOnly(bool readOnly) {
            if (!m_dataBlock) {
                m_dataBlock = std::shared_ptr<DataBlock>(new DataBlock(), [](DataBlock* block) {
                    delete[] block->m_data;
                    delete block;
                });
            }
            m_dataBlock->m_readOnly = readOnly;
        }
        private:
        struct DataBlock {
            char* m_data{nullptr};
            std::size_t m_size{0};
            bool m_readOnly{false};
        };
        std::shared_ptr<DataBlock> m_dataBlock;
    };
}