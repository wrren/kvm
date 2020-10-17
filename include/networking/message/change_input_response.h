#ifndef KVM_NETWORKING_CHANGE_INPUT_RESPONSE_H
#define KVM_NETWORKING_CHANGE_INPUT_RESPONSE_H

#include <networking/message.h>
#include <networking/message/types.h>
#include <display/display.h>
#include <core/core.h>
#include <map>

namespace kvm {
    class ChangeInputResponse : public NetworkMessage {
    public:

        typedef std::map<Display, bool> ResultMap;

        /**
         * Default Constructor
         */
        ChangeInputResponse();

        /**
         * Initializing Constructor. Specifies the result of the sending computer's
         * attempt to switch each display's input.
         */
        ChangeInputResponse(const ResultMap& result);

        /**
         * Get the result map contained in this message.
         */
        const ResultMap& GetResultMap() const;

        /**
         * Serialize this message into the given buffer.
         */
        virtual bool Serialize(NetworkBuffer& buffer) const override;

        /**
         * Deserialize a message of this type out of the given buffer.
         */
        virtual bool Deserialize(NetworkBuffer& buffer) override;

    private:

        ResultMap m_result;
    };
}

#endif // KVM_NETWORKING_CHANGE_INPUT_RESPONSE_H