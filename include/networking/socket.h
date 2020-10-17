#ifndef KVM_NETWORKING_SOCKET_H
#define KVM_NETWORKING_SOCKET_H

#include <optional>
#include <string>
#include <core/core.h>
#include <platform/types.h>
#include <networking/buffer.h>

namespace kvm {
    class Socket {
    public:

        enum class SocketError {
            INITIALIZATION_ERROR,
            DNS_ERROR,
            CONNECT_ERROR,
            BIND_ERROR
        };

        enum class SocketState {
            DISCONNECTED,
            CONNECTED,
            LISTENING
        };

        typedef Result<SocketAddress, Socket::SocketError>  GetAddressResult;
        typedef std::optional<SocketError>                  ConnectResult;
        typedef std::optional<SocketError>                  ListenResult;
        typedef std::optional<Socket>                       AcceptResult;
        typedef std::string                                 HostName;

        /**
         * Default Constructor
         */
        Socket();

        /**
         * Construct a socket representing a connection accepted from a peer.
         */
        Socket(PlatformSocket socket, SocketAddress peerAddress);

        /**
         * Connect this socket to the given address.
         */
        ConnectResult Connect(const SocketAddress& address);

        /**
         * Bind this socket to the specified port and begin listening for incoming connections.
         */
        ListenResult Listen(uint16_t port);

        /**
         * Accept a new connection.
         */
        AcceptResult Accept() const;

        /**
         * Send data to the connected peer.
         */
        bool Send(const NetworkBuffer& buffer);

        /**
         * Receive data from the connected peer.
         */
        bool Receive(NetworkBuffer& buffer);

        /**
         * Disconnect this socket.
         */
        void Disconnect();

        /**
         * Get the current socket state.
         */
        SocketState GetState() const;

        /**
         * Get the socket address for an IP and port combination.
         */
        static GetAddressResult GetAddressForIP(int ip, uint16_t port);

        /**
         * Get the socket address for a hostname and port combination.
         */
        static GetAddressResult GetAddressForHostname(const HostName& hostname, uint16_t port);

        /**
         * Comparison Operator
         */
        bool operator==(const Socket& other) const;

        /**
         * Destructor
         */
        ~Socket();

    private:

        /// Socket Handle
        PlatformSocket m_socket;
        /// Peer address
        SocketAddress m_address;
        /// Current Socket State
        SocketState m_state;
    };
}

#endif // KVM_NETWORKING_SOCKET_H