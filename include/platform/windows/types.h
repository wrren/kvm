#ifndef KVM_PLATFORM_TYPES_WINDOWS_H
#define KVM_PLATFORM_TYPES_WINDOWS_H

#include <winsock2.h>
#include <windef.h>
#include <core/reference.h>

namespace kvm {
  typedef struct {
    HMONITOR        handle;
    DISPLAY_DEVICE  device;
  } PlatformDisplay;

  typedef struct {
    HWND messageWindow;
  } PlatformUSB;

  typedef struct PlatformSocketStruct {
    SOCKET          id;
  } PlatformSocket;

  /// Used to control global WSAData initialization and destruction
  static ReferenceCounter<WSADATA> PlatformSocketReferencesPlatformSocketReferences(
    []() -> WSAData {
      WSAData wsa;
      WSAStartup(MAKEWORD(2, 2), &wsa);
      return wsa;
    },
    [](WSAData& wsa) -> WSAData {
      WSACleanup();
      return wsa;
    }
  );

  typedef struct sockaddr_in SocketAddress;
}

#endif // KVM_PLATFORM_TYPES_WINDOWS_H