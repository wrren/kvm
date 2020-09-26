#ifndef KVM_PLATFORM_TYPES_MAC_H
#define KVM_PLATFORM_TYPES_MAC_H

#include <CoreGraphics/CoreGraphics.h>
#include <libusb.h>

namespace kvm {
    typedef struct {
        CGDirectDisplayID   id;
    } PlatformDisplay;

    typedef struct {
        libusb_context* context;
    } PlatformUSB;
}

#endif // KVM_PLATFORM_TYPES_MAC_H