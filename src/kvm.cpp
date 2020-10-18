#include <kvm.h>

namespace kvm {
  KVM::KVM(uint16_t listenPort) :
  m_cluster(listenPort)
  {
    m_monitor.AddListener(this);
    m_cluster.AddListener(this);
  }

  bool KVM::Initialize() {
    return m_monitor.Initialize() && m_cluster.Initialize();
  }

  std::vector<USBDevice> KVM::ListUSBDevices() {
    return m_monitor.ListConnectedDevices();
  }

  std::vector<Display> KVM::ListDisplays() {
    return Display::ListDisplays();
  }

  std::vector<Display> KVM::ListDisplaysWithNonPreferredInput() {
    auto displays = ListDisplays();
    std::vector<Display> results;

    for(auto input : m_inputs) {
      for(auto display : displays) {
        if(display == input.first && display.GetInput() != input.second) {
          results.push_back(display);
        }
      }
    }

    return results;
  }

  void KVM::SetDesiredInputs(const Display::InputMap& inputs) {
    m_inputs = inputs;
  }

  void KVM::SetTriggerDevice(const USBDevice& device) {
    m_device = device;
  }

  void KVM::AddNode(const std::string& hostname, uint16_t port) {
    m_cluster.AddNode(hostname, port);
  }

  void KVM::AddListener(KVM::Listener* listener) {
    m_listeners.erase(std::remove(m_listeners.begin(), m_listeners.end(), listener), m_listeners.end());
    m_listeners.push_back(listener);
  }

  void KVM::RemoveListener(KVM::Listener* listener) {
    m_listeners.erase(std::remove(m_listeners.begin(), m_listeners.end(), listener), m_listeners.end());
  }
  
  void KVM::OnDeviceConnected(const kvm::USBDevice& device) {
    if(device == m_device) {
      auto displays = ListDisplays();
      Display::InputMap changes;

      for(auto input : m_inputs) {
        for(auto display : displays) {
          if(display == input.first && display.GetInput() != input.second) {
            changes[display] = input.second;
          }
        }
      }

      for(auto listener : m_listeners) {
        listener->OnTriggerDeviceConnected(device);
      }

      if(changes.size() > 0) {
        m_cluster.RequestInputChange(changes);
        ChangeState(KVM::State::REQUESTING_INPUT);
        for(auto listener : m_listeners) {
          listener->OnDisplayInputChangesRequested(changes);
        }
      } else {
        ChangeState(KVM::State::ACTIVE);
      }
    }
  }

  void KVM::OnDeviceDisconnected(const kvm::USBDevice& device) {
    if(device == m_device) {
      ChangeState(KVM::State::INACTIVE);
    }
  }

  void KVM::OnNodeConnected(const Node& node) {
    for(auto listener : m_listeners) {
      listener->OnNodeConnected(node);
    }
  }

  void KVM::OnNodeDisconnected(const Node& node) {
    for(auto listener : m_listeners) {
      listener->OnNodeDisconnected(node);
    }
  }

  void KVM::OnInputChangeRequested(const Node& sender, const Display::InputMap& changes) {
    auto displays = ListDisplays();

    for(auto change : changes) {
      for(auto display : displays) {
        if(display == change.first) {
          display.SetInput(change.second);
        }
      }
    }
  }

  void KVM::OnInputChangeResponse(const Node& sender, const std::map<Display, bool>& results) {
    if(m_state == KVM::State::REQUESTING_INPUT && ListDisplaysWithNonPreferredInput().size() == 0) {
      ChangeState(KVM::State::ACTIVE);
    }
  }

  void KVM::ChangeState(KVM::State newState) {
    KVM::State oldState = m_state;
    m_state = newState;

    for(auto listener : m_listeners) {
      listener->OnStateChange(oldState, newState);
    }
  }

  void KVM::Pump() {
    m_cluster.Pump();
    m_monitor.CheckForDeviceEvents();
  }
}