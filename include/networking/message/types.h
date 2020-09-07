#ifndef KVM_NETWORKING_MESSAGE_TYPES_H
#define KVM_NETWORKING_MESSAGE_TYPES_H

#include <networking/message.h>

namespace kvm {
    enum class NetworkMessageType: NetworkMessage::Type {
        CHANGE_INPUT_REQUEST,
        CHANGE_INPUT_RESPONSE
    };
}

#endif // KVM_NETWORKING_MESSAGE_TYPES_H