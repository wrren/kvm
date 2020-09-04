#ifndef KVM_USB_MONITOR_H
#define KVM_USB_MONITOR_H

#include <usb/device.h>
#include <vector>
#include <optional>
#include <string>

namespace kvm {

  /**
   * Monitors for changes in connected USB devices.
   */
  class USBMonitor {
  public:

    /**
     * Initialize USB monitoring. Returns an optional that, if set, indicates that an error occurred and contains
     * a human-readable error string.
     */
    std::optional<std::string> Initialize();

    class Subscriber {
    public:

      /**
       * Called when a USB device is connected to this computer.
       */
      void OnDeviceConnected(const kvm::USBDevice& device);

      /**
       * Called when a USB device is disconnected from this computer.
       */
      void OnDeviceDisconnected(const kvm::USBDevice& device);
    };

    /**
     * Get a list of all connected USB devices.
     */
    std::vector<USBDevice> ListConnectedDevices();

    /**
     * Subscribe to USB device events.
     */
    void Subscribe(Subscriber* subscriber);

    /**
     *  Unsubscribe from USB device events.
     */
    void Unsubscribe(Subscriber* subscriber);

  private:

    /// Device event subscribers
    std::vector<Subscriber*> m_subscribers;
  };
}

#endif // KVM_USB_MONITOR_H