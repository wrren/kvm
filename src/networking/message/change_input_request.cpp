#include <networking/message/change_input_request.h>
#include <networking/message/types.h>
#include <kvm.h>

namespace kvm {
    ChangeInputRequest::ChangeInputRequest(const ChangeInputRequest::InputMap& map) :
    NetworkMessage(static_cast<NetworkMessage::Type>(NetworkMessageType::CHANGE_INPUT_REQUEST)),
    m_map(map)
    {}

    ChangeInputRequest::ChangeInputRequest() :
    NetworkMessage(static_cast<NetworkMessage::Type>(NetworkMessageType::CHANGE_INPUT_REQUEST))
    {}

    bool ChangeInputRequest::Deserialize(NetworkBuffer& buffer) {
        if(NetworkMessage::Deserialize(buffer)) {
            m_map.clear();

            uint8_t     size;
            DisplayID   displayID;
            uint8_t     inputID;

            buffer >> size;

            for(int i = 0; i < size && buffer.GetState() == NetworkBuffer::State::OK; i++) {
                buffer >> displayID >> inputID;
                m_map[displayID] = static_cast<Display::Input>(inputID);
            }
        }

        return buffer.GetState() == NetworkBuffer::State::OK;
    }

    bool ChangeInputRequest::Serialize(NetworkBuffer& buffer) const {
        if(NetworkMessage::Serialize(buffer)) {
            buffer << static_cast<uint32_t>(m_map.size());

            for(auto it = m_map.begin(); it != m_map.end(); ++it) {
                buffer << it->first << static_cast<uint8_t>(it->second);
            }
        }

        return buffer.GetState() == NetworkBuffer::State::OK;
    }
}