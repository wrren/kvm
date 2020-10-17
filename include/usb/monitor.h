#ifndef KVM_USB_MONITOR_H
#define KVM_USB_MONITOR_H

#include <mutex>
#include <vector>
#include <string>
#include <optional>
#include <core/core.h>
#include <usb/device.h>
#include <platform/types.h>

namespace kvm {
  /**
   * Monitors for changes in connected USB devices.
   */
  class USBMonitor {
  public:
    class Listener {
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
     * an enum value indicating the nature of the error.
     */
    bool Initialize();

    /**
     * Get a list of all connected USB devices.
     */
    std::vector<USBDevice> ListConnectedDevices();

    /**
     * Subscribe to USB device events.
     */
    void AddListener(Listener* subscriber);

    /**
     *  Unsubscribe from USB device events.
     */
    void RemoveListener(Listener* subscriber);

    /**
     * Check for new device connect or disconnect events.
     */
    void CheckForDeviceEvents();

    /**
     * Called when a new device is connected to the system
     */
    void OnDeviceConnected(const USBDevice& device);

    /**
     * Called when a device is disconnected from the system
     */
    void OnDeviceDisconnected(const USBDevice& device);

  private:

    /// Platform USB Context
    PlatformUSB m_usb;
    /// Device event listeners
    std::vector<Listener*> m_listeners;
    /// Connected Devices
    std::vector<USBDevice> m_devices;
    /// Controls access to the device list from other threads
    std::mutex m_deviceMutex;
  };
}

#endif // KVM_USB_MONITOR_H