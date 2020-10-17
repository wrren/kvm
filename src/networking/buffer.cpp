#include <networking/buffer.h>
#include <networking/serializable.h>
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

    NetworkBuffer::Buffer NetworkBuffer::GetBuffer() const {
        return m_buffer;
    }

    NetworkBuffer& NetworkBuffer::Reset() {
        m_state     = NetworkBuffer::State::OK;
        m_offset    = 0;
        return *this;
    }
    NetworkBuffer& NetworkBuffer::Reset(NetworkBuffer::Buffer buffer, NetworkBuffer::BufferSize bufferSize) {
        m_state     = NetworkBuffer::State::OK;
        m_offset    = 0;

        if(m_buffer != nullptr) {
            delete[] m_buffer;
        }

        if(bufferSize > 0) {
            m_buffer = new uint8_t[bufferSize];
            m_length = bufferSize;
            memcpy(m_buffer, buffer, bufferSize);
        }
        
        return *this;
    }

    NetworkBuffer& operator>>(NetworkBuffer& buffer, uint8_t& value) {
        return buffer.Deserialize(&value, sizeof(value));
    }
    NetworkBuffer& operator>>(NetworkBuffer& buffer, int8_t& value) {
        return buffer.Deserialize(&value, sizeof(value));
    }
    NetworkBuffer& operator>>(NetworkBuffer& buffer, uint16_t& value) {
        buffer.Deserialize(&value, sizeof(value));
        value = NetworkToHost(value);
        return buffer;
    }
    NetworkBuffer& operator>>(NetworkBuffer& buffer, int16_t& value) {
        buffer.Deserialize(&value, sizeof(value));
        value = NetworkToHost(value);
        return buffer;
    }
    NetworkBuffer& operator>>(NetworkBuffer& buffer, uint32_t& value) {
        buffer.Deserialize(&value, sizeof(value));
        value = NetworkToHost(value);
        return buffer;
    }
    NetworkBuffer& operator>>(NetworkBuffer& buffer, int32_t& value) {
        buffer.Deserialize(&value, sizeof(value));
        value = NetworkToHost(value);
        return buffer;
    }
    NetworkBuffer& operator>>(NetworkBuffer& buffer, bool& value) {
        uint8_t b;
        buffer >> b;
        value = b;
        return buffer;
    }
    NetworkBuffer& operator>>(NetworkBuffer& buffer, std::string& value) {
        uint32_t length;
        buffer >> length;
        if(buffer.GetState() == NetworkBuffer::State::OK && length <= MAX_STRING_LENGTH) {
            char* str = new char[length];
            buffer.Deserialize(str, length);
            value.assign(str, length);
        } else {
            buffer.m_state = NetworkBuffer::State::ERROR_OVERFLOW;
        }
        return buffer;
    }
    NetworkBuffer& operator>>(NetworkBuffer& buffer, Serializable& value) {
        value.Deserialize(buffer);
        return buffer;
    }


    NetworkBuffer& operator<<(NetworkBuffer& buffer, uint8_t value) {
        return buffer.Serialize(&value, sizeof(value));
    }
    NetworkBuffer& operator<<(NetworkBuffer& buffer, int8_t value) {
        return buffer.Serialize(&value, sizeof(value));
    }
    NetworkBuffer& operator<<(NetworkBuffer& buffer, uint16_t value) {
        auto swapped = HostToNetwork(value);
        return buffer.Serialize(&swapped, sizeof(swapped));
    }
    NetworkBuffer& operator<<(NetworkBuffer& buffer, int16_t value) {
        auto swapped = HostToNetwork(value);
        return buffer.Serialize(&swapped, sizeof(swapped));
    }
    NetworkBuffer& operator<<(NetworkBuffer& buffer, uint32_t value) {
        auto swapped = HostToNetwork(value);
        return buffer.Serialize(&swapped, sizeof(swapped));
    }
    NetworkBuffer& operator<<(NetworkBuffer& buffer, int32_t value) {
        auto swapped = HostToNetwork(value);
        return buffer.Serialize(&swapped, sizeof(swapped));
    }
    NetworkBuffer& operator<<(NetworkBuffer& buffer, bool value) {
        auto swapped = HostToNetwork(static_cast<uint8_t>(value));
        return buffer.Serialize(&swapped, sizeof(swapped));
    }
    NetworkBuffer& operator<<(NetworkBuffer& buffer, const std::string& value) {
        if(value.length() <= MAX_STRING_LENGTH) {
            buffer << static_cast<uint32_t>(value.length());
            return buffer.Serialize(reinterpret_cast<const void*>(value.c_str()), value.length());
        } else {
            buffer.m_state = NetworkBuffer::State::ERROR_OVERFLOW;
        }     
    }
    NetworkBuffer& operator<<(NetworkBuffer& buffer, const Serializable& value) {
        value.Serialize(buffer);
        return buffer;    
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

    NetworkBuffer::operator bool() const {
        return m_state == NetworkBuffer::State::OK;
    }

    NetworkBuffer::~NetworkBuffer() {
        delete[] m_buffer;
    }
}