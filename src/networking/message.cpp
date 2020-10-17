#include <networking/message.h>

namespace kvm {
    NetworkMessage::NetworkMessage(Type type) :
    m_type(type)
    {}

    NetworkMessage::Type NetworkMessage::GetType() const {
        return m_type;
    }

    bool NetworkMessage::IsContainedIn(NetworkMessage::Type type, NetworkBuffer& buffer) {
        NetworkMessage::Type bufferType;
        return buffer.Peek(bufferType) && type == bufferType;
    }

    bool NetworkMessage::Serialize(NetworkBuffer& buffer) const {
        return buffer << m_type;
    }

    bool NetworkMessage::Deserialize(NetworkBuffer& buffer) {
        return buffer >> m_type;
    }
}