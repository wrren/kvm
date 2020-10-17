#include <networking/socket.h>

namespace kvm {
  Socket::SocketState Socket::GetState() const {
    return m_state;
  }
}