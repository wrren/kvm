#include <usb/monitor.h>
#include <algorithm>
#include <iostream>
#include <libusb.h>

#define DEVICE_NAME_MAX_LENGTH 256

std::optional<kvm::USBDevice> ToUSBDevice(libusb_device* device) {
  libusb_device_handle* handle;

  if(libusb_open(device, &handle) < 0) {
    return std::optional<kvm::USBDevice>();
  }

  libusb_device_descriptor descriptor;
  libusb_get_device_descriptor(device, &descriptor);

  unsigned char description[DEVICE_NAME_MAX_LENGTH];
  libusb_get_string_descriptor_ascii(handle, descriptor.iProduct, description, DEVICE_NAME_MAX_LENGTH);
  libusb_close(handle);

  return std::optional<kvm::USBDevice>(kvm::USBDevice(std::string((const char*) description), descriptor.idVendor, descriptor.idProduct));
}

static int LIBUSB_CALL hotplug_callback(libusb_context* ctx, libusb_device* dev, libusb_hotplug_event event, void* monitor) {
  auto device = ToUSBDevice(dev);

  if(device.has_value()) {
    auto subscribers = reinterpret_cast<kvm::USBMonitor*>(monitor)->ListSubscribers();

    std::cout << "Device Connected: " << device.value().GetDescription() << std::endl;

    for(auto subscriber : subscribers) {
      subscriber->OnDeviceConnected(device.value());
    }
  }

  return 0;
}

static int LIBUSB_CALL hotplug_callback_detach(libusb_context* ctx, libusb_device* dev, libusb_hotplug_event event, void *monitor) {
  auto device = ToUSBDevice(dev);

  if(device.has_value()) {
    auto subscribers = reinterpret_cast<kvm::USBMonitor*>(monitor)->ListSubscribers();

    std::cout << "Device Disconnected: " << device.value().GetDescription() << std::endl;

    for(auto subscriber : subscribers) {
      subscriber->OnDeviceDisconnected(device.value());
    }
  } else {
    std::cout << "Device Disconnect Event with Unrecognized Device" << std::endl;
  }

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

  std::optional<std::string> USBMonitor::Initialize() {
    if(m_ctx == nullptr) {
      if(libusb_init(&m_ctx) != 0) {
        m_ctx = nullptr;
        return std::optional<std::string>("Failed to initialize libusb.");
      }

      if(!libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG)) {
        libusb_exit(m_ctx);
        return std::optional<std::string>("This platform does not support hotplug capabilities, cannot monitor for device connection events.");
      }

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
        return std::optional<std::string>("Failed to register device connection callback with libusb.");
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
        return std::optional<std::string>("Failed to register device disconnection callback with libusb.");
      }
    }

    return std::optional<std::string>();
  }

  std::vector<USBDevice> USBMonitor::ListConnectedDevices() {
    std::vector<USBDevice> devices;

    libusb_device** list;
    auto length = libusb_get_device_list(m_ctx, &list);

    for(int i = 0; i < length; i++) {
      auto device = ToUSBDevice(list[i]);

      if(device.has_value()) {
        devices.push_back(device.value());
      }
    }

    libusb_free_device_list(list, 1);


    return devices;
  }

  const std::vector<USBMonitor::Subscriber*> USBMonitor::ListSubscribers() const {
    return m_subscribers;
  }

  void USBMonitor::CheckForDeviceEvents() {
    libusb_handle_events(m_ctx);
  }
}