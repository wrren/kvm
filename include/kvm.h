#ifndef KVM_H
#define KVM_H

#include <core/core.h>
#include <display/display.h>
#include <usb/monitor.h>
#include <usb/device.h>

namespace kvm {
    class KVM : public USBMonitor::Subscriber {
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
         * Set the input to which connected displays should be set when this machine is active.
         */
        void SetDesiredInput(Display::Input input);

        /**
         * Set the device that, when connected, marks this machine as active and triggers a display input change.
         */
        void SetTriggerDevice(const USBDevice& device);

        /**
         * Add a node to the KVM cluster. This machine will request display input changes by communicating with
         * cluster nodes.
         */
        void AddNode(const std::string& hostname);

        /**
         * Watch for connection and disconnection events for the given USB device
         * and, on device connection, attempt to switch connected displays to the given
         * input.
         */
        void Watch(const USBDevice& device, Display::Input input);

        /**
         * Called when a USB device is connected to this computer.
         */
        virtual void OnDeviceConnected(const kvm::USBDevice& device) override;

        /**
         * Called when a USB device is disconnected from this computer.
         */
        virtual void OnDeviceDisconnected(const kvm::USBDevice& device) override;

    private:

        enum class State {
            ACTIVE,
            INACTIVE
        };
        
        /// Current State
        State m_state;
        /// USB Monitor. Used to watch for changes in connected devices.
        USBMonitor m_monitor;
        /// Device to watch for connectivity changes.
        USBDevice m_device;
        /// Input to switch display(s) to when a connection event occurs.
        Display::Input m_input;
        /// Nodes to which this
        std::vector<std::string> m_nodes;
    };
}

#endif // KVM_H