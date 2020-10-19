#include <networking/buffer.h>
#include <networking/serializable.h>
#include <core/core.h>
#include <cstring>

#define MAX_STRING_LENGTH 1024

namespace kvm {
    NetworkBuffer::NetworkBuffer() :
    m_offset(0),
    m_length(0),
    m_state(NetworkBuffer::State::OK)
    {}

    NetworkBuffer::NetworkBuffer(const NetworkBuffer& other) :
    m_buffer(other.m_buffer),
    m_state(other.m_state),
    m_offset(other.m_offset),
    m_length(other.m_length)
    {}

    NetworkBuffer::State NetworkBuffer::GetState() const {
        return m_state;
    }

    NetworkBuffer::Offset NetworkBuffer::GetCapacity() const {
        return m_buffer.max_size();
    }

    NetworkBuffer::Offset NetworkBuffer::GetOffset() const {
        return m_offset;
    }

    uint8_t* NetworkBuffer::GetBuffer() {
        return m_buffer.data();
    }

    const uint8_t* NetworkBuffer::GetBuffer() const {
        return m_buffer.data();
    }

    NetworkBuffer::Offset NetworkBuffer::GetSize() const {
        return m_length;
    }

    NetworkBuffer& NetworkBuffer::Append(uint8_t* data, size_t size) {
        if(m_offset + size <= m_buffer.max_size()) {
            memcpy(m_buffer.data() + m_offset, data, size);
            m_offset += size;
            m_length = m_offset;
        } else {
            m_state = NetworkBuffer::State::ERROR_OVERFLOW;
        }

        return *this;
    }

    NetworkBuffer& NetworkBuffer::Reset() {
        m_state     = NetworkBuffer::State::OK;
        m_offset    = 0;
        return *this;
    }
    NetworkBuffer& NetworkBuffer::Reset(uint8_t* data, size_t size) {
        if(size <= m_buffer.max_size()) {
            memcpy(m_buffer.data(), data, size);
            m_state     = NetworkBuffer::State::OK;
            m_offset    = 0;
            m_length    = size;
        } else {
            m_state = NetworkBuffer::State::ERROR_OVERFLOW;
        }
        
        return *this;
    }
    NetworkBuffer& NetworkBuffer::Reset(NetworkBuffer::Buffer buffer) {
        m_state     = NetworkBuffer::State::OK;
        m_offset    = 0;
        m_buffer    = buffer;
        
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

    NetworkBuffer& NetworkBuffer::Deserialize(void* out, NetworkBuffer::Offset size) {
        if(m_state == NetworkBuffer::State::OK) {
            if(m_offset + size <= m_length) {
                memcpy(out, m_buffer.data() + m_offset, size);
                m_offset += size;
            } else {
                m_state = NetworkBuffer::State::ERROR_OVERFLOW;
            }
        }
        return *this;
    }

    NetworkBuffer& NetworkBuffer::Serialize(const void* in, NetworkBuffer::Offset size) {
        if(m_state == NetworkBuffer::State::OK) {
            if(m_offset + size < m_buffer.max_size()) {
                memcpy(m_buffer.data() + m_offset, in, size);
                m_offset += size;
                m_length = m_offset;
            } else {
                m_state = NetworkBuffer::State::ERROR_OVERFLOW;
            }
        }
        return *this;
    }

    NetworkBuffer::operator bool() const {
        return m_state == NetworkBuffer::State::OK;
    }
}