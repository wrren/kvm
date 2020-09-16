#include <display/display.h>
#include <platform/types.h>
#include <iostream>
#include <WinUser.h>

namespace kvm {
  Display::List Display::ListDisplays() {
    Display::List displays;

    PDISPLAY_DEVICEA device;
    DWORD index = 0;
    while(EnumDisplayDevices(nullptr, index, device, EDD_GET_DEVICE_INTERFACE_NAME)) {
    }

    return displays;
  }

  bool Display::SetInput(Display::Input input) {
    return true;
  }
}