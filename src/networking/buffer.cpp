#include <networking/buffer.h>
#include <core/core.h>
#include <cstring>

#define MAX_STRING_LENGTH 1024

namespace kvm {
    NetworkBuffer::NetworkBuffer(NetworkBuffer::Buffer buffer, NetworkBuffer::BufferSize size) :
    NetworkBuffer(size) {
        if(m_buffer != nullptr) {
            memcpy(m_buffer, buffer, size);
        }
    }

    NetworkBuffer::NetworkBuffer(NetworkBuffer::BufferSize size) :
    m_offset(0),
    m_length(size),
    m_buffer(nullptr),
    m_state(NetworkBuffer::State::OK) {
        if(m_length > 0) {
            m_buffer = new uint8_t[size];
        }
    }

    NetworkBuffer::State NetworkBuffer::GetState() const {
        return m_state;
    }

    NetworkBuffer::BufferSize NetworkBuffer::GetOffset() const {
        return m_offset;
    }

    NetworkBuffer& NetworkBuffer::Reset() {
        m_state     = NetworkBuffer::State::OK;
        m_offset    = 0;
        return *this;
    }

    NetworkBuffer& operator>>(NetworkBuffer& message, uint8_t& value) {
        return message.Deserialize(&value, sizeof(value));
    }
    NetworkBuffer& operator>>(NetworkBuffer& message, int8_t& value) {
        return message.Deserialize(&value, sizeof(value));
    }
    NetworkBuffer& operator>>(NetworkBuffer& message, uint16_t& value) {
        message.Deserialize(&value, sizeof(value));
        value = NetworkToHost(value);
        return message;
    }
    NetworkBuffer& operator>>(NetworkBuffer& message, int16_t& value) {
        message.Deserialize(&value, sizeof(value));
        value = NetworkToHost(value);
        return message;
    }
    NetworkBuffer& operator>>(NetworkBuffer& message, uint32_t& value) {
        message.Deserialize(&value, sizeof(value));
        value = NetworkToHost(value);
        return message;
    }
    NetworkBuffer& operator>>(NetworkBuffer& message, int32_t& value) {
        message.Deserialize(&value, sizeof(value));
        value = NetworkToHost(value);
        return message;
    }
    NetworkBuffer& operator>>(NetworkBuffer& message, bool& value) {
        uint8_t b;
        message >> b;
        value = b;
        return message;
    }
    NetworkBuffer& operator>>(NetworkBuffer& message, std::string& value) {
        uint32_t length;
        message >> length;
        if(message.GetState() == NetworkBuffer::State::OK && length <= MAX_STRING_LENGTH) {
            char* str = new char[length];
            message.Deserialize(str, length);
            value.assign(str, length);
        } else {
            message.m_state = NetworkBuffer::State::ERROR_OVERFLOW;
        }
        return message;
    }

    NetworkBuffer& operator<<(NetworkBuffer& message, uint8_t value) {
        return message.Serialize(&value, sizeof(value));
    }
    NetworkBuffer& operator<<(NetworkBuffer& message, int8_t value) {
        return message.Serialize(&value, sizeof(value));
    }
    NetworkBuffer& operator<<(NetworkBuffer& message, uint16_t value) {
        auto swapped = HostToNetwork(value);
        return message.Serialize(&swapped, sizeof(swapped));
    }
    NetworkBuffer& operator<<(NetworkBuffer& message, int16_t value) {
        auto swapped = HostToNetwork(value);
        return message.Serialize(&swapped, sizeof(swapped));
    }
    NetworkBuffer& operator<<(NetworkBuffer& message, uint32_t value) {
        auto swapped = HostToNetwork(value);
        return message.Serialize(&swapped, sizeof(swapped));
    }
    NetworkBuffer& operator<<(NetworkBuffer& message, int32_t value) {
        auto swapped = HostToNetwork(value);
        return message.Serialize(&swapped, sizeof(swapped));
    }
    NetworkBuffer& operator<<(NetworkBuffer& message, bool value) {
        auto swapped = HostToNetwork(static_cast<uint8_t>(value));
        return message.Serialize(&swapped, sizeof(swapped));
    }
    NetworkBuffer& operator<<(NetworkBuffer& message, const std::string& value) {
        if(value.length() <= MAX_STRING_LENGTH) {
            message << static_cast<uint32_t>(value.length());
            return message.Serialize(reinterpret_cast<const void*>(value.c_str()), value.length());
        } else {
            message.m_state = NetworkBuffer::State::ERROR_OVERFLOW;
        }     
    }

    NetworkBuffer& NetworkBuffer::Deserialize(void* out, NetworkBuffer::BufferSize size) {
        if(m_state == NetworkBuffer::State::OK) {
            if(m_offset + size < m_length) {
                memcpy(out, m_buffer + m_offset, size);
                m_buffer += size;
            } else {
                m_state = NetworkBuffer::State::ERROR_OVERFLOW;
            }
        }
        return *this;
    }

    NetworkBuffer& NetworkBuffer::Serialize(const void* in, NetworkBuffer::BufferSize size) {
        if(m_state == NetworkBuffer::State::OK) {
            if(m_offset + size < m_length) {
                memcpy(m_buffer + m_offset, in, size);
                m_buffer += size;
            } else {
                m_state = NetworkBuffer::State::ERROR_OVERFLOW;
            }
        }
        return *this;
    }

    NetworkBuffer::~NetworkBuffer() {
        delete[] m_buffer;
    }
}