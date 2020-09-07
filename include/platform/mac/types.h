#ifndef KVM_PLATFORM_TYPES_MAC_H
#define KVM_PLATFORM_TYPES_MAC_H

#include <CoreGraphics/CoreGraphics.h>

namespace kvm {
    typedef CGDirectDisplayID   DisplayID;
    typedef uint32_t            DisplaySerialNumber;
    typedef uint32_t            DisplayVendorID;
}

#endif // KVM_PLATFORM_TYPES_MAC_H