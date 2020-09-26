#ifndef KVM_NETWORKING_SERIALIZABLE_H
#define KVM_NETWORKING_SERIALIZABLE_H

#include <networking/buffer.h>

namespace kvm {
    class Serializable {
    public:

        virtual bool Serialize(NetworkBuffer& buffer) const {
            return true;
        }

        virtual bool Deserialize(NetworkBuffer& buffer) {
            return true;
        }
    };
}

#endif // KVM_NETWORKING_SERIALIZABLE_H