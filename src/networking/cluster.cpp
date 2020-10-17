#include <networking/cluster.h>
#include <networking/message.h>
#include <networking/message/types.h>
#include <networking/message/change_input_request.h>
#include <networking/message/change_input_response.h>

namespace kvm {
  Cluster::Cluster(uint16_t listenPort) :
  m_listenPort(listenPort)
  {}

  bool Cluster::Initialize() {
    return m_socket.Listen(m_listenPort).has_value() == false;
  }

  void Cluster::AddNode(const std::string& hostname, uint16_t port) {
    Node node(hostname, port);
    node.AddListener(this);
    m_nodes.push_back(node);
  }

  void Cluster::RequestInputChange(const std::map<Display, Display::Input>& changes) {
    ChangeInputRequest request(changes);
    NetworkBuffer buffer(1024);
    if(request.Serialize(buffer)) {
      for(auto node : m_nodes) {
        node.Send(buffer);
      }
    }
  }

  void Cluster::RespondToInputChangeRequest(const Node& sender, const std::map<Display, bool>& changes) {
    ChangeInputResponse response(changes);
    NetworkBuffer buffer(1024);
    if(response.Serialize(buffer)) {
      for(auto node : m_nodes) {
        node.Send(buffer);
      }
    }
  }

  void Cluster::AddListener(Cluster::Listener* listener) {
    m_listeners.erase(std::remove(m_listeners.begin(), m_listeners.end(), listener), m_listeners.end());
    m_listeners.push_back(listener);
  }

  void Cluster::RemoveListener(Cluster::Listener* listener) {
    m_listeners.erase(std::remove(m_listeners.begin(), m_listeners.end(), listener), m_listeners.end());
  }

  void Cluster::Pump() {
    auto socket = m_socket.Accept();

    if(socket) {
      Node node(socket.value());
      node.AddListener(this);
      m_nodes.push_back(Node(socket.value()));

      for(auto listener : m_listeners) {
        listener->OnNodeConnected(node);
      }
    }

    for(auto node : m_nodes) {
      node.Pump();
    }
  }

  void Cluster::OnNodeConnected(const Node& node) {
    for(auto listener : m_listeners) {
      listener->OnNodeConnected(node);
    }
  }

  void Cluster::OnNodeDisconnected(const Node& node) {
    for(auto listener : m_listeners) {
      listener->OnNodeDisconnected(node);
    }
  }

  void Cluster::OnMessageReceived(Node& sender, NetworkBuffer& buffer) {
    if(NetworkMessage::IsContainedIn(static_cast<NetworkMessage::Type>(NetworkMessageType::CHANGE_INPUT_REQUEST), buffer)) {
      ChangeInputRequest request;
      if(request.Deserialize(buffer)) {
        for(auto listener : m_listeners) {
          listener->OnInputChangeRequested(sender, request.GetInputMap());
        }
      }
    }
  }
}