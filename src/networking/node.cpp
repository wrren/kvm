#include <networking/node.h>
#include <networking/message/heartbeat.h>
#include <networking/message/types.h>

namespace kvm {
  Node::Node(const std::string& hostname, uint16_t port) {
    auto address = Socket::GetAddressForHostname(hostname, port);
    if(address.DidSucceed()) {
      m_address = address.GetValue();
    }
  }

  Node::Node(Socket socket) :
  m_socket(socket) 
  {}

  SocketAddress Node::GetAddress() const {
    return m_address;
  }

  bool Node::IsConnected() const {
    return m_socket.GetState() == Socket::SocketState::CONNECTED;
  }

  bool Node::Send(NetworkBuffer& buffer) {
    return m_socket.Send(buffer);
  }

  void Node::AddListener(Node::Listener* listener) {
    m_listeners.erase(std::remove(m_listeners.begin(), m_listeners.end(), listener), m_listeners.end());
    m_listeners.push_back(listener);
  }

  void Node::RemoveListener(Node::Listener* listener) {
    m_listeners.erase(std::remove(m_listeners.begin(), m_listeners.end(), listener), m_listeners.end());
  }

  void Node::Pump() {
    if(m_socket.GetState() != Socket::SocketState::CONNECTED) {
      auto result = m_socket.Connect(m_address);
      if(result.has_value() == false) {
        for(auto listener : m_listeners) {
          listener->OnNodeConnected(*this);
        }
        m_lastSeen = std::chrono::system_clock::now();
      }

      return;
    }

    NetworkBuffer buffer;
    
    if(m_spacer(std::chrono::seconds(5))) {
      Heartbeat heartbeat;
      heartbeat.Serialize(buffer);
      m_socket.Send(buffer);
    }

    buffer.Reset();

    if(m_socket.Receive(buffer)) {
      if(NetworkMessage::IsContainedIn(static_cast<NetworkMessage::Type>(NetworkMessageType::HEARTBEAT), buffer) == false) {
        for(auto listener : m_listeners) {
          listener->OnMessageReceived(*this, buffer);
        }
      } else {
        m_lastSeen = std::chrono::system_clock::now();
      }
    }

    if( m_socket.GetState() != Socket::SocketState::CONNECTED || 
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - m_lastSeen) >= std::chrono::seconds(15)) {
      for(auto listener : m_listeners) {
        listener->OnNodeDisconnected(*this);
      }
    }
  }
}