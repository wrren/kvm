#ifndef KVM_NODE_H
#define KVM_NODE_H

#include <string>
#include <vector>
#include <networking/socket.h>
#include <core/time.h>

namespace kvm {
    class Node {
    public:

        class Listener {
        public:

            /**
             * Called when a node is connected
             */
            virtual void OnNodeConnected(const Node& node)
            {}

            /**
             * Called when we lose our connection to a node
             */
            virtual void OnNodeDisconnected(const Node& node)
            {}

            /**
             * Called when a non-heartbeat message is received from the network.
             */
            virtual void OnMessageReceived(Node& sender, NetworkBuffer& buffer) = 0;
        };

        /**
         * Default Constructor
         */
        Node(const std::string& hostname, uint16_t port);

        /**
         * Construct a Node from a pre-connected socket.
         */
        Node(Socket socket);

        /**
         * Get this node's network address
         */
        SocketAddress GetAddress() const;

        /**
         * Determine whether this node is connected.
         */
        bool IsConnected() const;

        /**
         * Send a message to this node.
         */
        bool Send(NetworkBuffer& buffer);

        /**
         * Add an event listener.
         */
        void AddListener(Listener* listener);

        /**
         * Remove an event listener.
         */
        void RemoveListener(Listener* listener);

        /**
         * Send heartbeat messages, attempt connects and reconnects, etc.
         */
        void Pump();

    private:

        /// Spaces out pump periods
        TimeSpacer m_spacer;
        /// Listeners
        std::vector<Listener*> m_listeners;
        /// Node Address
        SocketAddress m_address;
        /// Socket
        Socket m_socket;
        /// Time at which the last heartbeat was received from this node, or since we connected to it
        std::chrono::time_point<std::chrono::system_clock> m_lastSeen;
    };
}

#endif // KVM_NODE_H