#ifndef KVM_H
#define KVM_H

#include <core/core.h>
#include <display/display.h>
#include <usb/device.h>

namespace kvm {
    class KVM {
    public:

        /**
         * Initialize the KVM system.
         */
        bool Initialize();

        /**
         * Get a list of connected USB devices.
         */
        std::vector<USBDevice> ListUSBDevices();

        /**
         * Get a list of connected displays.
         */
        std::vector<Display> ListDisplays();

        /**
         * Watch for connection and disconnection events for the given USB device
         * and, on device connection, attempt to switch connected displays to the given
         * input.
         */
        void Watch(const USBDevice& device, Display::Input input);

    private:

        /// USB Monitor. Used to watch for changes in connected devices.
        USBMonitor m_monitor;
        /// Device to watch for connectivity changes.
        USBDevice m_device;
        /// Input to switch display(s) to when a connection event occurs.
        Display::Input m_input;
    };
}

#endif // KVM_H