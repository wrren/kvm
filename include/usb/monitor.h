#ifndef KVM_USB_MONITOR_H
#define KVM_USB_MONITOR_H

#include <usb/device.h>
#include <vector>
#include <optional>
#include <string>

struct libusb_context;

namespace kvm {

  /**
   * Monitors for changes in connected USB devices.
   */
  class USBMonitor {
  public:
    class Subscriber {
    public:

      /**
       * Called when a USB device is connected to this computer.
       */
      virtual void OnDeviceConnected(const kvm::USBDevice& device) = 0;

      /**
       * Called when a USB device is disconnected from this computer.
       */
      virtual void OnDeviceDisconnected(const kvm::USBDevice& device) = 0;
    };

    /**
     * Default Constructor
     */
    USBMonitor();

    /**
     * Initialize USB monitoring. Returns an optional that, if set, indicates that an error occurred and contains
     * a human-readable error string.
     */
    std::optional<std::string> Initialize();

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

    /**
     * List USB event subscribers
     */
    const std::vector<Subscriber*> ListSubscribers() const;

    /**
     * Check for new device connect or disconnect events.
     */
    void CheckForDeviceEvents();

  private:

    /// Device event subscribers
    std::vector<Subscriber*> m_subscribers;

    /// LibUSB Context
    libusb_context* m_ctx;
  };
}

#endif // KVM_USB_MONITOR_H