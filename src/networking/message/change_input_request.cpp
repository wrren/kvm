#include <networking/message/change_input_request.h>
#include <networking/message/types.h>
#include <kvm.h>

namespace kvm {
    ChangeInputRequest::ChangeInputRequest(const Display::InputMap& map) :
    NetworkMessage(static_cast<NetworkMessage::Type>(NetworkMessageType::CHANGE_INPUT_REQUEST)),
    m_map(map)
    {}

    ChangeInputRequest::ChangeInputRequest() :
    NetworkMessage(static_cast<NetworkMessage::Type>(NetworkMessageType::CHANGE_INPUT_REQUEST))
    {}

    const Display::InputMap& ChangeInputRequest::GetInputMap() const {
        return m_map;
    }

    bool ChangeInputRequest::Deserialize(NetworkBuffer& buffer) {
        if(NetworkMessage::Deserialize(buffer)) {
            m_map.clear();

            uint8_t size;
            Display display;
            uint8_t input;

            buffer >> size;

            for(int i = 0; i < size && buffer; i++) {
                buffer >> display >> input;
                if(buffer) {
                    m_map[display] = static_cast<Display::Input>(input);
                }
            }
        }

        return buffer;
    }

    bool ChangeInputRequest::Serialize(NetworkBuffer& buffer) const {
        if(NetworkMessage::Serialize(buffer)) {
            buffer << static_cast<uint32_t>(m_map.size());

            for(auto it = m_map.begin(); it != m_map.end(); ++it) {
                buffer << it->first << static_cast<uint8_t>(it->second);
            }
        }

        return buffer;
    }
}