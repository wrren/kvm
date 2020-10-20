#ifndef KVM_PLATFORM_TYPES_UNIX_H
#define KVM_PLATFORM_TYPES_UNIX_H

#include <sys/socket.h>
#include <arpa/inet.h>

namespace kvm {
    typedef int                 PlatformSocket;
    typedef struct sockaddr_in  SocketAddress;
}

#endif // KVM_PLATFORM_TYPES_UNIX_H