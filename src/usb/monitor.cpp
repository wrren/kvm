#include <usb/monitor.h>
#include <algorithm>

namespace kvm {
  void USBMonitor::AddListener(USBMonitor::Listener* listener) {
    RemoveListener(listener);
    m_listeners.push_back(listener);
  }

  void USBMonitor::RemoveListener(USBMonitor::Listener* listener) {
    m_listeners.erase(std::remove(m_listeners.begin(), m_listeners.end(), listener), m_listeners.end());
  }

  void USBMonitor::OnDeviceConnected(const USBDevice& device) {
    m_deviceMutex.lock();
    m_devices.erase(std::remove(m_devices.begin(), m_devices.end(), device), m_devices.end());
    m_devices.push_back(device);
    for(auto listener : m_listeners) {
      listener->OnDeviceConnected(device);
    }
    m_deviceMutex.unlock();
  }

  void USBMonitor::OnDeviceDisconnected(const USBDevice& device) {
    m_deviceMutex.lock();
    for(auto d : m_devices) {
      if(d == device) {
        for(auto listener : m_listeners) {
          listener->OnDeviceDisconnected(d);
        }
      }
    }
    m_devices.erase(std::remove(m_devices.begin(), m_devices.end(), device), m_devices.end());
    m_deviceMutex.unlock();
  }
}