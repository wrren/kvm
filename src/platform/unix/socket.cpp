#include <networking/socket.h>
#include <core/core.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAX_BACKLOG_LENGTH 64

namespace kvm {
    Socket::Socket() :
    m_socket(-1),
    m_state(Socket::SocketState::DISCONNECTED)
    {}

    Socket::Socket(PlatformSocket socket, SocketAddress address) :
    m_state(Socket::SocketState::CONNECTED),
    m_socket(socket),
    m_address(address)
    {}

    Socket::ConnectResult Socket::Connect(const SocketAddress& address) {
        if(m_state != Socket::SocketState::DISCONNECTED) {
            Disconnect();
        }

        m_socket = socket(AF_INET, SOCK_STREAM, 0);

        if(m_socket == -1) {
            return Socket::ConnectResult(Socket::SocketError::INITIALIZATION_ERROR);
        }

        if(connect(m_socket, reinterpret_cast<const struct sockaddr*>(&address), sizeof(address)) < 0) {
            m_socket = -1;
            return Socket::ConnectResult(Socket::SocketError::CONNECT_ERROR);
        }

        m_state = Socket::SocketState::CONNECTED;

        return Socket::ConnectResult();
    }

    Socket::ListenResult Socket::Listen(uint16_t port) {
        if(m_state != Socket::SocketState::DISCONNECTED) {
            Disconnect();
        }

        m_socket = socket(AF_INET, SOCK_STREAM, 0);

        if(m_socket == -1) {
            return Socket::ListenResult(Socket::SocketError::INITIALIZATION_ERROR);
        }

        SocketAddress address;
        address.sin_family      = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port        = HostToNetwork(port);

        if(bind(m_socket, reinterpret_cast<struct sockaddr*>(&address), sizeof(address)) < 0) {
            Disconnect();
            return Socket::ListenResult(Socket::SocketError::BIND_ERROR);
        }
        
        listen(m_socket, MAX_BACKLOG_LENGTH);

        m_state = Socket::SocketState::LISTENING;
        return Socket::ListenResult();;
    }

    Socket::AcceptResult Socket::Accept() const {
        if(m_state == Socket::SocketState::LISTENING) {
            fd_set readSet;
            struct timeval timeout;
            FD_ZERO(&readSet);
            FD_SET(m_socket, &readSet);

            timeout.tv_sec  = 0;
            timeout.tv_usec = 0;

            if(select(m_socket + 1, &readSet, NULL, NULL, &timeout) > 0) {
                SocketAddress   clientAddress;
                socklen_t       clientAddressLength = sizeof(SocketAddress);
                PlatformSocket  clientSocket = accept(m_socket, (struct sockaddr*) &clientAddress, &clientAddressLength);

                if(clientSocket != -1) {
                    return Socket::AcceptResult(Socket(clientSocket, clientAddress));
                }
            }
        }
        return Socket::AcceptResult();
    }

    bool Socket::Send(const NetworkBuffer& buffer) {
        if(m_state == Socket::SocketState::CONNECTED && buffer.GetOffset() > 0 && buffer) {
            return send(m_socket, buffer.GetBuffer(), buffer.GetOffset(), 0) > 0;
        }
        return false;
    }

    bool Socket::Receive(NetworkBuffer& buffer) {
        if(m_state == Socket::SocketState::CONNECTED) {
            fd_set readSet;
            struct timeval timeout;
            FD_ZERO(&readSet);
            FD_SET(m_socket, &readSet);

            timeout.tv_sec  = 0;
            timeout.tv_usec = 0;

            if(select(m_socket + 1, &readSet, NULL, NULL, &timeout) > 0) {
                uint8_t receiveBuffer[2048];            
                int receiveSize = recv(m_socket, (char*) receiveBuffer, 2048, 0);

                if(receiveSize == -1) {
                    m_state = Socket::SocketState::DISCONNECTED;
                    return false;
                }

                buffer.Reset(receiveBuffer, receiveSize);

                return true;
            }
        }
        return false;
    }

    void Socket::Disconnect() {
        if(m_state != Socket::SocketState::DISCONNECTED) {
            close(m_socket);
            m_socket    = -1;
            m_state     = Socket::SocketState::DISCONNECTED;
        }
    }
    
    Socket::GetAddressResult Socket::GetAddressForIP(int ip, uint16_t port) {
        struct sockaddr_in address;
        address.sin_addr.s_addr     = ip;
        address.sin_family          = AF_INET;
        address.sin_port            = HostToNetwork(port);

        return Socket::GetAddressResult(address);
    }

    Socket::GetAddressResult Socket::GetAddressForHostname(const Socket::HostName& hostname, uint16_t port) {
        struct hostent*     entry;
        struct in_addr**    addresses;

        if((entry = gethostbyname(hostname.c_str())) == NULL) {
            return Socket::GetAddressResult(Socket::SocketError::DNS_ERROR);
        }

        if(entry->h_length == 0) {
            return Socket::GetAddressResult(Socket::SocketError::DNS_ERROR);
        }
        
        addresses = reinterpret_cast<struct in_addr**>(entry->h_addr_list);

        struct sockaddr_in address;
        address.sin_addr.s_addr     = addresses[0]->s_addr;
        address.sin_family          = AF_INET;
        address.sin_port            = HostToNetwork(port);

        return Socket::GetAddressResult(address);
    }

    Socket::~Socket()
    {}
}