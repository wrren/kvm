#ifndef KVM_H
#define KVM_H

#include <core/core.h>
#include <display/display.h>
#include <usb/monitor.h>
#include <usb/device.h>
#include <networking/cluster.h>

namespace kvm {
    class KVM : public Cluster::Listener,
                public USBMonitor::Listener {
    public:

        enum class State {
            INACTIVE,
            REQUESTING_INPUT,
            ACTIVE
        };

        class Listener {
        public:

            /**
             * Called when a new node connects to the cluster or we establish a connection to a node.
             */
            virtual void OnNodeConnected(const Node& node)
            {}

            /**
             * Called when a node disconnects from the cluster or times out.
             */
            virtual void OnNodeDisconnected(const Node& node)
            {}

            /**
             * Called when KVM transitions between states
             */
            virtual void OnStateChange(State previousState, State newState) = 0;

            /**
             * Called when the trigger USB device has been connected.
             */
            virtual void OnTriggerDeviceConnected(const USBDevice& device) = 0;

            /**
             * Called when this node requests a set of display inputs to be changed.
             */
            virtual void OnDisplayInputChangesRequested(const Display::InputMap& changes) = 0;
        };

        /**
         * Construct a KVM instance that will listen for node connections on the given port.
         */
        KVM(uint16_t listenPort);

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
         * Get a list of connected displays whose active display inputs differ from those that are preferred by 
         * this node.
         */
        std::vector<Display> ListDisplaysWithNonPreferredInput();

        /**
         * Set the inputs to which connected displays should be set when this machine is active.
         */
        void SetDesiredInputs(const Display::InputMap& inputs);

        /**
         * Set the device that, when connected, marks this machine as active and triggers a display input change.
         */
        void SetTriggerDevice(const USBDevice& device);

        /**
         * Add a node to the KVM cluster. This machine will request display input changes by communicating with
         * cluster nodes.
         */
        void AddNode(const std::string& hostname, uint16_t port);

        /**
         * Add an event listener.
         */
        void AddListener(Listener* listener);

        /**
         * Remove an event listener.
         */
        void RemoveListener(Listener* listener);

        /**
         * Watch for USB events and react to messages from connected nodes.
         */
        void Pump();

        /**
         * Called when a USB device is connected to this computer.
         */
        virtual void OnDeviceConnected(const kvm::USBDevice& device) override;

        /**
         * Called when a USB device is disconnected from this computer.
         */
        virtual void OnDeviceDisconnected(const kvm::USBDevice& device) override;

        /**
         * Called when a new node connects to the cluster or we establish a connection to a node.
         */
        virtual void OnNodeConnected(const Node& node) override;

        /**
         * Called when a node disconnects from the cluster or times out.
         */
        virtual void OnNodeDisconnected(const Node& node) override;

        /**
         * Called when a connected node requests an input change.
         */
        virtual void OnInputChangeRequested(const Node& sender, const Display::InputMap& changes) override;

        /**
         * Called when we receive an input change response from a node.
         */
        virtual void OnInputChangeResponse(const Node& sender, const std::map<Display, bool>& results) override;

    private:

        /**
         * Change the object state and inform listeners.
         */
        void ChangeState(State newState);
        
        /// Event Listeners
        std::vector<Listener*> m_listeners;
        /// Node Cluster
        Cluster m_cluster;
        /// Current State
        State m_state;
        /// USB Monitor. Used to watch for changes in connected devices.
        USBMonitor m_monitor;
        /// Device to watch for connectivity changes.
        USBDevice m_device;
        /// Input to switch display(s) to when a connection event occurs.
        Display::InputMap m_inputs;
        /// Nodes to which this
        std::vector<std::string> m_nodes;
    };
}

#endif // KVM_H