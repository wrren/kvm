#include <usb/monitor.h>
#include <algorithm>
#include <iostream>
#include <libusb.h>
#include <thread>

#define DEVICE_NAME_MAX_LENGTH 256

kvm::Result<kvm::USBDevice, kvm::USBMonitor::DeviceConversionError> ToUSBDevice(libusb_device* device, bool getDescription = true) {
  std::string description = "";
  libusb_device_descriptor descriptor;
  libusb_get_device_descriptor(device, &descriptor);

  if(getDescription) {
    libusb_device_handle* handle;
    auto result = libusb_open(device, &handle);
    if(result < 0) {
      return kvm::Result<kvm::USBDevice, kvm::USBMonitor::DeviceConversionError>(kvm::USBMonitor::DEVICE_OPEN_FAILURE);
    }
  
    unsigned char desc[DEVICE_NAME_MAX_LENGTH];
    libusb_get_string_descriptor_ascii(handle, descriptor.iProduct, desc, DEVICE_NAME_MAX_LENGTH);
    description = reinterpret_cast<const char*>(desc);
    
    libusb_close(handle);
  }

  return kvm::Result<kvm::USBDevice, kvm::USBMonitor::DeviceConversionError>(kvm::USBDevice(description, descriptor.idVendor, descriptor.idProduct));
}

static int LIBUSB_CALL hotplug_callback(libusb_context* ctx, libusb_device* dev, libusb_hotplug_event event, void* monitor) {
  std::thread converter([dev, monitor]() {
    auto result = ToUSBDevice(dev);

    if(result.DidSucceed()) {
      auto device = result.GetValue();
      reinterpret_cast<kvm::USBMonitor*>(monitor)->OnDeviceConnected(device);
    }
  });

  converter.detach();

  return 0;
}

static int LIBUSB_CALL hotplug_callback_detach(libusb_context* ctx, libusb_device* dev, libusb_hotplug_event event, void *monitor) {
  std::thread converter([dev, monitor]() {
    auto result = ToUSBDevice(dev, false);

    if(result.DidSucceed()) {
      auto device = result.GetValue();
      reinterpret_cast<kvm::USBMonitor*>(monitor)->OnDeviceDisconnected(device);
    }
  });

  converter.detach();

  return 0;
}

namespace kvm {
  USBMonitor::USBMonitor() :
  m_ctx(nullptr)
  {}

  void USBMonitor::Subscribe(USBMonitor::Subscriber* subscriber) {
    Unsubscribe(subscriber);
    m_subscribers.push_back(subscriber);
  }

  void USBMonitor::Unsubscribe(USBMonitor::Subscriber* subscriber) {
    m_subscribers.erase(std::remove(m_subscribers.begin(), m_subscribers.end(), subscriber), m_subscribers.end());
  }

  bool USBMonitor::Initialize() {
    if(m_ctx == nullptr) {
      if(libusb_init(&m_ctx) != 0) {
        m_ctx = nullptr;
        return false;
      }

      if(!libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG)) {
        libusb_exit(m_ctx);
        m_mode = USBMonitor::Mode::DEVICE_POLLING;
      } else {
        auto rc = libusb_hotplug_register_callback(
          m_ctx, 
          LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED, 
          LIBUSB_HOTPLUG_NO_FLAGS, 
          LIBUSB_HOTPLUG_MATCH_ANY, 
          LIBUSB_HOTPLUG_MATCH_ANY, 
          LIBUSB_HOTPLUG_MATCH_ANY, 
          hotplug_callback, 
          this,
          NULL
        );

        if(rc != LIBUSB_SUCCESS) {
          libusb_exit(m_ctx);
          return false;
        }

        rc = libusb_hotplug_register_callback(
          m_ctx, 
          LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, 
          LIBUSB_HOTPLUG_NO_FLAGS, 
          LIBUSB_HOTPLUG_MATCH_ANY, 
          LIBUSB_HOTPLUG_MATCH_ANY, 
          LIBUSB_HOTPLUG_MATCH_ANY, 
          hotplug_callback_detach, 
          this,
          NULL
        );

        if(rc != LIBUSB_SUCCESS) {
          libusb_exit(m_ctx);
          return false;
        }
        m_mode = USBMonitor::Mode::HOTPLUG_DETECTION;
      }

      ListConnectedDevices();
    }

    return true;
  }

  std::vector<USBDevice> USBMonitor::ListConnectedDevices() {
    m_deviceMutex.lock();

    m_devices.clear();
    libusb_device** list;
    auto length = libusb_get_device_list(m_ctx, &list);

    for(int i = 0; i < length; i++) {
      auto result = ToUSBDevice(list[i]);

      if(result.DidSucceed()) {
        m_devices.push_back(result.GetValue());
      }
    }

    m_deviceMutex.unlock();
    return m_devices;
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

  void USBMonitor::CheckForDeviceEvents() {
    if(m_ctx != nullptr) {
      if(m_mode == USBMonitor::Mode::HOTPLUG_DETECTION) {
        libusb_handle_events(m_ctx);
      } else {
        auto previous = m_devices;
        auto next     = ListConnectedDevices();
        std::vector<USBDevice> difference;

        if(previous.size() != next.size()) {
          std::set_difference(previous.begin(), previous.end(), next.begin(), next.end(), std::inserter(difference, difference.begin()));
          if(previous.size() > next.size()) {
            for(auto disconnected : difference) {
              for(auto subscriber : m_subscribers) {
                subscriber->OnDeviceDisconnected(disconnected);
              }
            }
          } else {
            for(auto connected : difference) {
              for(auto subscriber : m_subscribers) {
                subscriber->OnDeviceConnected(connected);
              }
            }
          }
        }
      }
    }
  }
}