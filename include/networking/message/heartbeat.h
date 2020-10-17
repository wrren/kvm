#ifndef KVM_NETWORKING_HEARTBEAT_H
#define KVM_NETWORKING_HEARTBEAT_H

#include <networking/message.h>
#include <map>

namespace kvm {
    class Heartbeat : public NetworkMessage {
    public:

        /**
         * Default Constructor
         */
        Heartbeat();
    };
}

#endif // KVM_NETWORKING_HEARTBEAT_H