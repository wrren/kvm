#include <kvm.h>

namespace kvm {
  bool KVM::Initialize() {
    if(m_monitor.Initialize()) {

    }
    return true;
  }

  std::vector<USBDevice> KVM::ListUSBDevices() {
    return m_monitor.ListConnectedDevices();
  }

  std::vector<Display> KVM::ListDisplays() {
    return Display::ListDisplays();
  }

  void KVM::SetDesiredInput(Display::Input input) {

  }

  void KVM::SetTriggerDevice(const USBDevice& device) {

  }

  void KVM::AddNode(const std::string& hostname) {

  }
  
  void KVM::OnDeviceConnected(const kvm::USBDevice& device) {

  }

  void KVM::OnDeviceDisconnected(const kvm::USBDevice& device) {

  }

  void KVM::Watch(const USBDevice& device, Display::Input input) {

  }
}