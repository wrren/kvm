#ifndef KVM_NETWORKING_CHANGE_INPUT_REQUEST_H
#define KVM_NETWORKING_CHANGE_INPUT_REQUEST_H

#include <networking/message.h>
#include <display/display.h>
#include <map>

namespace kvm {
    class ChangeInputRequest : public NetworkMessage {
    public:

        /**
         * Create a request input message that requests that the specified 
         * displays be set to the provided corresponding inputs.
         */
        ChangeInputRequest(const Display::InputMap& map);

        /**
         * Default Constructor
         */
        ChangeInputRequest();

        /**
         * Set the input map contained in this message.
         */
        void SetInputMap(const Display::InputMap& map);

        /**
         * Get the input map contained in this message.
         */
        const Display::InputMap& GetInputMap() const;

        /**
         * Serialize this message into the given buffer.
         */
        virtual bool Serialize(NetworkBuffer& buffer) const override;

        /**
         * Deserialize a message of this type out of the given buffer.
         */
        virtual bool Deserialize(NetworkBuffer& buffer) override;

    private:

        /// Input Map
        Display::InputMap m_map;
    };
}

#endif // KVM_NETWORKING_CHANGE_INPUT_REQUEST_H