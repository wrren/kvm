#include <networking/socket.h>

namespace kvm {
  Socket::SocketState Socket::GetState() const {
    return m_state;
  }

  SocketAddress Socket::GetAddress() const {
    return m_address;
  }
}