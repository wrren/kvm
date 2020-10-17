#include <platform/functions.h>
#include <WinSock2.h>
#include <string>

#pragma comment(lib, "legacy_stdio_definitions.lib")
#pragma comment(lib, "ws2_32.lib")

namespace kvm {
    uint32_t HostToNetwork(uint32_t in) {
        return htonl(in);
    }
    uint16_t HostToNetwork(uint16_t in) {
        return htons(in);
    }
    int32_t HostToNetwork(int32_t in) {
        return htonl(in);
    }
    int16_t HostToNetwork(int16_t in) {
        return htons(in);
    }

    uint32_t NetworkToHost(uint32_t in) {
        return ntohl(in);
    }
    uint16_t NetworkToHost(uint16_t in) {
        return ntohs(in);
    }
    int32_t NetworkToHost(int32_t in) {
        return ntohl(in);
    }
    int16_t NetworkToHost(int16_t in) {
        return ntohs(in);
    }

    std::string AddressToString(SocketAddress address) {
        return std::string(inet_ntoa(address.sin_addr));
    }
}