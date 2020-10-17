#ifndef KVM_CLUSTER_H
#define KVM_CLUSTER_H

#include <map>
#include <vector>
#include <chrono>
#include <display/display.h>
#include <networking/socket.h>
#include <networking/node.h>

namespace kvm {
    class Cluster : public Node::Listener {
    public:

        class Listener {
        public:

            /**
             * Called when an input change request message is received.
             */
            virtual void OnInputChangeRequested(const Node& sender, const std::map<Display, Display::Input>& changes) = 0;

            /**
             * Called when we receive an input change response from a node.
             */
            virtual void OnInputChangeResponse(const Node& sender, const std::map<Display, bool>& results) = 0;

            /**
             * Called when a node connects to the cluster
             */
            virtual void OnNodeConnected(const Node& node)
            {}

            /**
             * Called when a node disconnects from the cluster
             */
            virtual void OnNodeDisconnected(const Node& node)
            {}
        };

        /**
         * Default Constructor
         */
        Cluster(uint16_t listenPort);

        /**
         * Initialize the cluster.
         */
        bool Initialize();

        /**
         * Add a new node to the cluster.
         */
        void AddNode(const std::string& hostname, uint16_t port);

        /**
         * Request that connected nodes trigger an input change to the specified display input.
         */
        void RequestInputChange(const std::map<Display, Display::Input>& changes);

        /**
         * Respond to an input change request by indicating the changes that succeeded.
         */
        void RespondToInputChangeRequest(const Node& sender, const std::map<Display, bool>& result);

        /**
         * Add an event listener.
         */
        void AddListener(Listener* listener);

        /**
         * Remove an event listener.
         */
        void RemoveListener(Listener* listener);

        /**
         * Send heartbeats, attempt reconnects to nodes, etc.
         */
        void Pump();

        /**
         * Called when a node is connected.
         */
        virtual void OnNodeConnected(const Node& node) override;

        /**
         * Called when we lose our connection to a given node.
         */
        virtual void OnNodeDisconnected(const Node& node) override;

        /**
         * Called by Node objects when they receive a non-heartbeat message.
         */
        virtual void OnMessageReceived(Node& sender, NetworkBuffer& buffer) override;

    private:

        /// Socket Listen Port
        uint16_t m_listenPort;
        /// Listen Socket
        Socket m_socket;
        /// Connected Nodes
        std::vector<Node> m_nodes;
        /// Event Listeners
        std::vector<Listener*> m_listeners;
        /// Indicates when each connected node was last seen
        std::map<Node, std::chrono::time_point<std::chrono::system_clock>> m_lastSeen;
    };
}

#endif // KVM_CLUSTER_H