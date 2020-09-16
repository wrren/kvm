#include <networking/message/change_input_response.h>
#include <networking/message/types.h>
#include <kvm.h>

namespace kvm {
    ChangeInputResponse::ChangeInputResponse(const ChangeInputResponse::ResultMap& result) :
    NetworkMessage(static_cast<NetworkMessage::Type>(NetworkMessageType::CHANGE_INPUT_RESPONSE)),
    m_result(result)
    {}

    ChangeInputResponse::ChangeInputResponse() :
    NetworkMessage(static_cast<NetworkMessage::Type>(NetworkMessageType::CHANGE_INPUT_RESPONSE))
    {}

    const ChangeInputResponse::ResultMap& ChangeInputResponse::GetResultMap() const {
        return m_result;
    }

    bool ChangeInputResponse::Deserialize(NetworkBuffer& buffer) {
        if(NetworkMessage::Deserialize(buffer)) {
            m_result.clear();

            uint8_t     size;
            uint8_t     index;
            uint8_t     result;

            buffer >> size;

            for(int i = 0; i < size && buffer.GetState() == NetworkBuffer::State::OK; i++) {
                buffer >> index >> result;
                m_result[index] = static_cast<ChangeInputResponse::Result>(result);
            }
        }

        return buffer.GetState() == NetworkBuffer::State::OK;
    }

    bool ChangeInputResponse::Serialize(NetworkBuffer& buffer) const {
        if(NetworkMessage::Serialize(buffer)) {
            buffer << static_cast<uint32_t>(m_result.size());

            for(auto it = m_result.begin(); it != m_result.end(); ++it) {
                buffer << it->first << static_cast<uint8_t>(it->second);
            }
        }

        return buffer.GetState() == NetworkBuffer::State::OK;
    }
}