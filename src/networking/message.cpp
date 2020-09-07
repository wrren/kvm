#include <networking/message.h>

namespace kvm {
    NetworkMessage::NetworkMessage(Type type) :
    m_type(type)
    {}

    NetworkMessage::Type NetworkMessage::GetType() const {
        return m_type;
    }

    bool NetworkMessage::IsContainedIn(NetworkBuffer& buffer) const {
        NetworkMessage::Type type;
        return buffer.Peek(type) && type == m_type;
    }

    bool NetworkMessage::Serialize(NetworkBuffer& buffer) const {
        buffer << m_type;
        return buffer.GetState() == NetworkBuffer::State::OK;
    }

    bool NetworkMessage::Deserialize(NetworkBuffer& buffer) {
        buffer >> m_type;
        return buffer.GetState() == NetworkBuffer::State::OK;
    }
}