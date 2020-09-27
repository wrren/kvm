#include <usb/monitor.h>

namespace kvm {
  USBMonitor::USBMonitor() {

  }

  bool USBMonitor::Initialize() {
    return true;
  }

  std::vector<USBDevice> USBMonitor::ListConnectedDevices() {
    std::vector<USBDevice> devices;

    return devices;
  }

  void USBMonitor::CheckForDeviceEvents() {
    
  }
}