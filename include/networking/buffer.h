#ifndef KVM_NETWORKING_BUFFER_H
#define KVM_NETWORKING_BUFFER_H

#include <cstdint>
#include <string>
#include <array>

namespace kvm {
    class Serializable;

    /**
     * Container class that handles serialization to and from buffers that are sent over the network
     * through Socket instances.
     */
    class NetworkBuffer {
    public:

        typedef std::array<uint8_t, 2048>   Buffer;
        typedef Buffer::size_type           Offset;

        enum class State {
            OK,
            ERROR_OVERFLOW
        };

        /**
         * Default Constructor
         */
        NetworkBuffer();

        /**
         * Copy Constructor
         */
        NetworkBuffer(const NetworkBuffer& buffer);

        /**
         * Get the network message's current state.
         */
        State GetState() const;

        /**
         * Get the buffer's capacity
         */
        Offset GetCapacity() const;

        /**
         * Get the current buffer offset.
         */
        Offset GetOffset() const;

        /**
         * Get the underlying buffer.
         */
        uint8_t* GetBuffer();
        const uint8_t* GetBuffer() const;

        /**
         * Get the amount of data that's been serialized into this buffer.
         */
        Offset GetSize() const;

        /**
         * Append data at the current offset
         */
        NetworkBuffer& Append(uint8_t* data, size_t size);

        /**
         * Resets the internal offset value for this buffer. Subsequent reads and writes will
         * occur at the beginning of the buffer's storage space.
         */
        NetworkBuffer& Reset();
        NetworkBuffer& Reset(uint8_t* data, size_t size);
        NetworkBuffer& Reset(Buffer buffer);

        /**
         * Attempt to peek at a value inside the buffer. This does not advance the internal offset.
         * Returns false if the buffer is incapable of containing a value of the given type.
         */
        template<typename T>
        bool Peek(T& value) {
            auto offset = m_offset;
            *this >> value;
            m_offset = offset;
            return m_state == State::OK;
        }

        /**
         * Output operator. Streams a value of the given type from the network message.
         */
        friend NetworkBuffer& operator>>(NetworkBuffer& message, uint8_t& value);
        friend NetworkBuffer& operator>>(NetworkBuffer& message, int8_t& value);
        friend NetworkBuffer& operator>>(NetworkBuffer& message, uint16_t& value);
        friend NetworkBuffer& operator>>(NetworkBuffer& message, int16_t& value);
        friend NetworkBuffer& operator>>(NetworkBuffer& message, uint32_t& value);
        friend NetworkBuffer& operator>>(NetworkBuffer& message, int32_t& value);
        friend NetworkBuffer& operator>>(NetworkBuffer& message, bool& value);
        friend NetworkBuffer& operator>>(NetworkBuffer& message, std::string& value);
        friend NetworkBuffer& operator>>(NetworkBuffer& message, Serializable& value);

        /**
         * Input operator. Streams a value of the given type into the network message.
         */
        friend NetworkBuffer& operator<<(NetworkBuffer& message, uint8_t value);
        friend NetworkBuffer& operator<<(NetworkBuffer& message, int8_t value);
        friend NetworkBuffer& operator<<(NetworkBuffer& message, uint16_t value);
        friend NetworkBuffer& operator<<(NetworkBuffer& message, int16_t value);
        friend NetworkBuffer& operator<<(NetworkBuffer& message, uint32_t value);
        friend NetworkBuffer& operator<<(NetworkBuffer& message, int32_t value);
        friend NetworkBuffer& operator<<(NetworkBuffer& message, bool value);
        friend NetworkBuffer& operator<<(NetworkBuffer& message, const std::string& value);
        friend NetworkBuffer& operator<<(NetworkBuffer& message, const Serializable& value);

        /**
         * Boolean operator. Returns true if the network buffer is in the OK state, false otherwise.
         */
        operator bool() const;

    private:

        /**
         * Deserialize the specified number of bytes out of the buffer and advance the offset.
         */
        NetworkBuffer& Deserialize(void* out, Offset size);

        /**
         * Serialize the specified number of bytes into the buffer and advance the offset.
         */
        NetworkBuffer& Serialize(const void* in, Offset size);

        /// Message Buffer
        Buffer m_buffer;
        /// Current Offset
        Offset m_offset;
        /// Contained Data Length
        Offset m_length;
        /// Current State
        State m_state;
    };
}

#endif // KVM_NETWORKING_BUFFER_H