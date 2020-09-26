#include <usb/monitor.h>
#include <algorithm>
#include <iostream>
#include <libusb.h>
#include <thread>





namespace kvm {
  void USBMonitor::Subscribe(USBMonitor::Subscriber* subscriber) {
    Unsubscribe(subscriber);
    m_subscribers.push_back(subscriber);
  }

  void USBMonitor::Unsubscribe(USBMonitor::Subscriber* subscriber) {
    m_subscribers.erase(std::remove(m_subscribers.begin(), m_subscribers.end(), subscriber), m_subscribers.end());
  }

  void USBMonitor::OnDeviceConnected(const USBDevice& device) {
    m_deviceMutex.lock();
    m_devices.erase(std::remove(m_devices.begin(), m_devices.end(), device), m_devices.end());
    m_devices.push_back(device);
    for(auto subscriber : m_subscribers) {
      subscriber->OnDeviceConnected(device);
    }
    std::cout << "Device Connected: " << device.GetDescription() << " (" << device.GetVendorID() << ":" << device.GetProductID() << ")" << std::endl;
    m_deviceMutex.unlock();
  }

  void USBMonitor::OnDeviceDisconnected(const USBDevice& device) {
    m_deviceMutex.lock();
    for(auto d : m_devices) {
      if(d == device) {
        std::cout << "Device Disconnected: " << d.GetDescription() << " (" << d.GetVendorID() << ":" << d.GetProductID() << ")" << std::endl;
        for(auto subscriber : m_subscribers) {
          subscriber->OnDeviceDisconnected(d);
        }
      }
    }
    m_devices.erase(std::remove(m_devices.begin(), m_devices.end(), device), m_devices.end());
    m_deviceMutex.unlock();
  }
}