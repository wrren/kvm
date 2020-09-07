#ifndef KVM_PLATFORM_UNIX_TYPES_H
#define KVM_PLATFORM_UNIX_TYPES_H

#include <sys/socket.h>
#include <arpa/inet.h>

namespace kvm {
    typedef int                 SocketID;
    typedef struct sockaddr_in  SocketAddress;
}

#endif // KVM_PLATFORM_UNIX_TYPES_H