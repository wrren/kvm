#ifndef KVM_PLATFORM_TYPES_WINDOWS_H
#define KVM_PLATFORM_TYPES_WINDOWS_H

#include <winsock2.h>
#include <windef.h>

namespace kvm {
  typedef struct {
    HMONITOR        handle;
    DISPLAY_DEVICE  device;
  } PlatformDisplay;

  typedef struct {
    HWND messageWindow;
  } PlatformUSB;

  typedef struct {
    SOCKET id;
  } PlatformSocket;

  typedef struct sockaddr_in  SocketAddress;
}

#endif // KVM_PLATFORM_TYPES_WINDOWS_H