#ifndef KVM_NODE_H
#define KVM_NODE_H

#include <string>
#include <networking/socket.h>

namespace kvm {
    class Node {
    public:

        /**
         * Default Constructor
         */
        Node(const std::string& hostname);

        

    private:

        /// Node Hostname
        std::string m_hostname;
        /// Socket
        Socket m_socket;
        /// Indicates whether we're currently connected to this node
        bool m_connected;
    };
}

#endif // KVM_NODE_H