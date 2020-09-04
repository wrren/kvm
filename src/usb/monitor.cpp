#include <usb/monitor.h>
#include <algorithm>
#include <libusb.h>

namespace kvm {
  void USBMonitor::Subscribe(USBMonitor::Subscriber* subscriber) {
    Unsubscribe(subscriber);
    m_subscribers.push_back(subscriber);
  }

  void USBMonitor::Unsubscribe(USBMonitor::Subscriber* subscriber) {
    m_subscribers.erase(std::remove(m_subscribers.begin(), m_subscribers.end(), subscriber), m_subscribers.end());
  }

  std::optional<std::string> USBMonitor::Initialize() {
    libusb_hotplug_callback_handle callback_handles[2];
    int rc = libusb_init(NULL);

    if(rc < 0) {
      return std::optional<std::string>(std::string("Failed to initialize libusb: ") + libusb_error_name(rc));
    }

    if (!libusb_has_capability (LIBUSB_CAP_HAS_HOTPLUG)) {
      return std::optional<std::string>("USB hotplug detection is unsupported.");
    }
  }

  std::vector<USBDevice> USBMonitor::ListConnectedDevices() {
    std::vector<USBDevice> devices;



    return devices;
  }
}