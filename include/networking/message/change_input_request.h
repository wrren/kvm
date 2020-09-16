#ifndef KVM_NETWORKING_CHANGE_INPUT_REQUEST_H
#define KVM_NETWORKING_CHANGE_INPUT_REQUEST_H

#include <networking/message.h>
#include <display/display.h>
#include <map>

namespace kvm {
    class ChangeInputRequest : NetworkMessage {
    public:

        typedef std::map<Display::Index, Display::Input> InputMap;

        /**
         * Create a request input message that requests that the specified 
         * displays be set to the provided corresponding inputs.
         */
        ChangeInputRequest(const InputMap& map);

        /**
         * Default Constructor
         */
        ChangeInputRequest();

        /**
         * Set the input map contained in this message.
         */
        void SetInputMap(const InputMap& map);

        /**
         * Get the input map contained in this message.
         */
        const InputMap& GetInputMap() const;

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
        InputMap m_map;
    };
}

#endif // KVM_NETWORKING_CHANGE_INPUT_REQUEST_H