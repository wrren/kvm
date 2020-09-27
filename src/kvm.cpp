#include <kvm.h>

namespace kvm {
  bool KVM::Initialize() {
    return m_monitor.Initialize();
  }

  std::vector<USBDevice> KVM::ListUSBDevices() {
    return m_monitor.ListConnectedDevices();
  }

  std::vector<Display> KVM::ListDisplays() {
    return Display::ListDisplays();
  }

  void KVM::Watch(const USBDevice& device, Display::Input input) {

  }
}