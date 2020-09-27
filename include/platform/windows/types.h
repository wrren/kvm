#ifndef KVM_PLATFORM_TYPES_WINDOWS_H
#define KVM_PLATFORM_TYPES_WINDOWS_H

#include <winsock2.h>

namespace kvm {
  typedef struct {
    HMONITOR handle;
  } PlatformDisplay;

  typedef struct {

  } PlatformUSB;

  typedef SOCKET SocketID;
  typedef struct sockaddr_in  SocketAddress;
}

#endif // KVM_PLATFORM_TYPES_WINDOWS_H