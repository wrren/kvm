#ifndef KVM_NETWORKING_SOCKET_H
#define KVM_NETWORKING_SOCKET_H

#include <kvm.h>
#include <optional>
#include <string>
#include <platform/types.h>

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
        typedef std::string                                 HostName;

        /**
         * Default Constructor
         */
        Socket();

        /**
         * Connect this socket to the given address.
         */
        ConnectResult Connect(const SocketAddress& address);

        /**
         * Bind this socket to the specified port and begin listening for incoming connections.
         */
        ListenResult Listen(uint16_t port);

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

    private:

        /// Socket Handle
        PlatformSocket m_socket;
        /// Current Socket State
        SocketState m_state;
    };
}

#endif // KVM_NETWORKING_SOCKET_H