#ifndef KVM_NETWORKING_BUFFER_H
#define KVM_NETWORKING_BUFFER_H

#include <cstdint>
#include <string>

#define DEFAULT_BUFFER_SIZE 2048

namespace kvm {
    class Serializable;

    /**
     * Container class that handles serialization to and from buffers that are sent over the network
     * through Socket instances.
     */
    class NetworkBuffer {
    public:

        typedef uint8_t*    Buffer;
        typedef size_t      BufferSize;

        enum class State {
            OK,
            ERROR_OVERFLOW
        };

        /**
         * Construct a NetworkBuffer using the contents of the given buffer.
         */
        NetworkBuffer(Buffer buffer, BufferSize size = DEFAULT_BUFFER_SIZE);

        /**
         * Construct a blank NetworkBuffer that can hold the given number of bytes.
         */
        NetworkBuffer(BufferSize size = DEFAULT_BUFFER_SIZE);

        /**
         * Get the network message's current state.
         */
        State GetState() const;

        /**
         * Get the current buffer offset.
         */
        BufferSize GetOffset() const;

        /**
         * Get the underlying buffer.
         */
        Buffer GetBuffer() const;


        /**
         * Resets the internal offset value for this buffer. Subsequent reads and writes will
         * occur at the beginning of the buffer's storage space.
         */
        NetworkBuffer& Reset();
        NetworkBuffer& Reset(Buffer buffer, BufferSize bufferSize);

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

        /**
         * Default Destructor
         */
        ~NetworkBuffer();

    private:

        /**
         * Deserialize the specified number of bytes out of the buffer and advance the offset.
         */
        NetworkBuffer& Deserialize(void* out, BufferSize size);

        /**
         * Serialize the specified number of bytes into the buffer and advance the offset.
         */
        NetworkBuffer& Serialize(const void* in, BufferSize size);

        /// Message Buffer
        Buffer m_buffer;
        /// Current Offset
        BufferSize m_offset;
        /// Buffer Length
        BufferSize m_length;
        /// Current State
        State m_state;
    };
}

#endif // KVM_NETWORKING_BUFFER_H