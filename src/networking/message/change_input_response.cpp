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

            uint8_t size;
            Display display;
            bool    result;

            buffer >> size;

            for(int i = 0; i < size && buffer; i++) {
                buffer >> display >> result;
                m_result[display] = result;
            }
        }

        return buffer;
    }

    bool ChangeInputResponse::Serialize(NetworkBuffer& buffer) const {
        if(NetworkMessage::Serialize(buffer)) {
            buffer << static_cast<uint32_t>(m_result.size());

            for(auto it = m_result.begin(); it != m_result.end(); ++it) {
                buffer << it->first << it->second;
            }
        }

        return buffer;
    }
}