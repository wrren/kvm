#include <networking/socket.h>

namespace kvm {
    ReferenceCounter<WSAData> PlatformSocketReferences(
      []() -> WSAData {
        WSAData wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);
        return wsa;
      },
      [](WSAData& wsa) -> WSAData {
        WSACleanup();
        return wsa;
      }
    );

    Socket::Socket() :
    m_state(Socket::SocketState::DISCONNECTED) {
      m_socket.id = INVALID_SOCKET;
    }

    Socket::Socket(PlatformSocket socket, SocketAddress address) :
    m_state(Socket::SocketState::CONNECTED),
    m_socket(socket),
    m_address(address)
    {}

    Socket::ConnectResult Socket::Connect(const SocketAddress& address) {
      if(m_state != Socket::SocketState::DISCONNECTED) {
        return Socket::ConnectResult(Socket::SocketError::INITIALIZATION_ERROR);
      }
      
      if(m_socket.id == INVALID_SOCKET) {
        ++PlatformSocketReferences;
        m_socket.id = socket(AF_INET, SOCK_STREAM, 0);

        if(m_socket.id == INVALID_SOCKET) {
          return Socket::ConnectResult(Socket::SocketError::INITIALIZATION_ERROR);
        }
      } else {
        Disconnect();
        return Connect(address);
      }

      if(connect(m_socket.id, (struct sockaddr*) &address, sizeof(address)) < 0) {
        return Socket::ConnectResult(Socket::SocketError::CONNECT_ERROR);
      }

      m_state   = Socket::SocketState::CONNECTED;
      m_address = address;

      return Socket::ConnectResult();
    }

    Socket::ListenResult Socket::Listen(uint16_t port) {
      SocketAddress address;
      address.sin_family            = AF_INET;
      address.sin_addr.S_un.S_addr  = INADDR_ANY;
      address.sin_port              = HostToNetwork(port);

      if(m_socket.id == INVALID_SOCKET) {
        ++PlatformSocketReferences;
        m_socket.id = socket(AF_INET, SOCK_STREAM, 0);

        if(m_socket.id == INVALID_SOCKET) {
          return Socket::ListenResult(Socket::SocketError::INITIALIZATION_ERROR);
        }
      } else {
        Disconnect();
        return Listen(port);
      }

      if(bind(m_socket.id, (struct sockaddr*) &address, sizeof(address)) == SOCKET_ERROR) {
        return Socket::ListenResult(Socket::SocketError::BIND_ERROR);
      }

      listen(m_socket.id, 10);

      m_state     = Socket::SocketState::LISTENING;
      m_address   = address;

      return Socket::ListenResult();
    }

    Socket::AcceptResult Socket::Accept() const {
      if(m_state == Socket::SocketState::LISTENING) {
        fd_set          readSet;
        SOCKET          client;
        SocketAddress   address;
        int             addressSize = sizeof(struct sockaddr_in);
        struct timeval  waitFor{0, 0};

        FD_ZERO(&readSet);
        FD_SET(m_socket.id, &readSet);

        if(select(0, &readSet, NULL, NULL, &waitFor) == SOCKET_ERROR) {
          return Socket::AcceptResult();
        }

        if(FD_ISSET(m_socket.id, &readSet)) {
          ++PlatformSocketReferences;
          client = accept(m_socket.id, (struct sockaddr*) &address, &addressSize);

          if(client != INVALID_SOCKET) {
            PlatformSocket newSocket;
            newSocket.id = client;
            return Socket::AcceptResult(Socket(newSocket, address));
          }
        }
      }

      return Socket::AcceptResult();
    }

    void Socket::Disconnect() {
      --PlatformSocketReferences;
      closesocket(m_socket.id);
      m_socket.id = INVALID_SOCKET;
    }

    bool Socket::Send(const NetworkBuffer& buffer) {
      if(m_state == Socket::SocketState::CONNECTED && buffer.GetOffset() > 0 && buffer.GetState() == NetworkBuffer::State::OK) {
        auto result = send(m_socket.id, (char*) buffer.GetBuffer(), buffer.GetOffset(), 0);
        if(result >= 0) {
          return true;
        } else {
          auto error = WSAGetLastError();
          m_state = Socket::SocketState::DISCONNECTED;
        }
      }
      return false;
    }

    bool Socket::Receive(NetworkBuffer& buffer) {
      if(m_state == Socket::SocketState::CONNECTED) {
        fd_set          readSet;
        struct timeval  waitFor{0, 0};
        
        FD_ZERO(&readSet);
        FD_SET(m_socket.id, &readSet);
        
        if(select(0, &readSet, NULL, NULL, &waitFor) == SOCKET_ERROR) {
          return false;
        }
      
        if(FD_ISSET(m_socket.id, &readSet)) {
          uint8_t receiveBuffer[2048];
          int receiveSize = recv(m_socket.id, (char*) receiveBuffer, 2048, 0);

          if(receiveSize == SOCKET_ERROR) {
            m_state = Socket::SocketState::DISCONNECTED;
            return false;
          }

          buffer.Reset(receiveBuffer, receiveSize);

          return true;
        }
      }
      return false;
    }

    Socket::GetAddressResult Socket::GetAddressForIP(int ip, uint16_t port) {
      SocketAddress address;

      address.sin_family            = AF_INET;
      address.sin_addr.S_un.S_addr  = ip;
      address.sin_port              = HostToNetwork(port);

      return Socket::GetAddressResult(address);
    }

    Socket::GetAddressResult Socket::GetAddressForHostname(const HostName& hostname, uint16_t port) {
      struct hostent*   hostEntry;
      struct in_addr**  addressList;

      if((hostEntry = gethostbyname(hostname.c_str())) == NULL) {
        return Socket::GetAddressResult(Socket::SocketError::DNS_ERROR);
      }

      addressList = (struct in_addr**) hostEntry->h_addr_list;

      if(addressList[0] != NULL) {
        SocketAddress address;
        address.sin_family  = AF_INET;
        address.sin_addr    = *addressList[0];
        address.sin_port    = HostToNetwork(port);

        return Socket::GetAddressResult(address);
      }

      return Socket::GetAddressResult(Socket::SocketError::DNS_ERROR);
    }

    bool Socket::operator==(const Socket& other) const {
      return m_socket.id == other.m_socket.id;
    }

    Socket::~Socket() {
    }
}