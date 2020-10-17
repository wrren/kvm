#ifndef KVM_PLATFORM_FUNCTIONS_H
#define KVM_PLATFORM_FUNCTIONS_H

#include <cstdint>
#include <platform/types.h>

namespace kvm {
    /**
     * Convert an integer's endianness from host to network byte order
     */
    uint32_t HostToNetwork(uint32_t in);
    uint16_t HostToNetwork(uint16_t in);
    int32_t HostToNetwork(int32_t in);
    int16_t HostToNetwork(int16_t in);

    /**
     * Convert an integer's endianness from network to host byte order.
     */
    uint32_t NetworkToHost(uint32_t in);
    uint16_t NetworkToHost(uint16_t in);
    int32_t NetworkToHost(int32_t in);
    int16_t NetworkToHost(int16_t in);

    /**
     * Convert a Socket Address to a string
     */
    std::string AddressToString(SocketAddress address);
}

#endif // KVM_PLATFORM_FUNCTIONS_H