#ifndef KVM_NETWORKING_MESSAGE_H
#define KVM_NETWORKING_MESSAGE_H

#include <networking/buffer.h>

namespace kvm {
    class NetworkMessage {
    public:
        typedef uint8_t Type;

        /**
         * Default Constructor. Specifies the type of this message.
         */
        NetworkMessage(Type type);

        /**
         * Get this message's unique type ID
         */
        Type GetType() const;

        /**
         * Determines whether the type field of the given buffer matches the given type.
         */
        static bool IsContainedIn(Type type, NetworkBuffer& buffer);

        /**
         * Serialize this message into the given buffer.
         */
        virtual bool Serialize(NetworkBuffer& buffer) const;

        /**
         * Deserialize a message of this type out of the given buffer.
         */
        virtual bool Deserialize(NetworkBuffer& buffer);

    private:

        /// Message type ID
        Type m_type;
    };
}

#endif // KVM_NETWORKING_MESSAGE_H