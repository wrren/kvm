#ifndef KVM_PLATFORM_TYPES_MAC_H
#define KVM_PLATFORM_TYPES_MAC_H

#include <CoreGraphics/CoreGraphics.h>

namespace kvm {
    struct DisplayID {
        CGDirectDisplayID   id;
    };
}

#endif // KVM_PLATFORM_TYPES_MAC_H