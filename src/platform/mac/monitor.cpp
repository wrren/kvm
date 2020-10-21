#include <usb/monitor.h>
#include <thread>
#include <libusb.h>

#define DEVICE_NAME_MAX_LENGTH 256

kvm::Result<kvm::USBDevice, bool> ToUSBDevice(libusb_device *device, bool getDescription = true)
{
    std::string description = "";
    libusb_device_descriptor descriptor;
    libusb_get_device_descriptor(device, &descriptor);

    if (getDescription)
    {
        libusb_device_handle *handle;
        auto result = libusb_open(device, &handle);
        if (result < 0)
        {
            return kvm::Result<kvm::USBDevice, bool>(false);
        }

        unsigned char desc[DEVICE_NAME_MAX_LENGTH];
        libusb_get_string_descriptor_ascii(handle, descriptor.iProduct, desc, DEVICE_NAME_MAX_LENGTH);
        description = reinterpret_cast<const char *>(desc);

        libusb_close(handle);
    }

    return kvm::Result<kvm::USBDevice, bool>(kvm::USBDevice(description, descriptor.idVendor, descriptor.idProduct));
}

static int LIBUSB_CALL hotplug_callback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *monitor)
{
    std::thread converter([dev, monitor]() {
        auto result = ToUSBDevice(dev);

        if (result.DidSucceed())
        {
            auto device = result.GetValue();
            reinterpret_cast<kvm::USBMonitor *>(monitor)->OnDeviceConnected(device);
        }
    });

    converter.detach();

    return 0;
}

static int LIBUSB_CALL hotplug_callback_detach(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *monitor)
{
    std::thread converter([dev, monitor]() {
        auto result = ToUSBDevice(dev, false);

        if (result.DidSucceed())
        {
            auto device = result.GetValue();
            reinterpret_cast<kvm::USBMonitor *>(monitor)->OnDeviceDisconnected(device);
        }
    });

    converter.detach();

    return 0;
}

namespace kvm
{
    USBMonitor::USBMonitor()
    {
        m_usb.context = nullptr;
    }

    bool USBMonitor::Initialize()
    {
        if (m_usb.context == nullptr)
        {
            if (libusb_init(&(m_usb.context)) != 0)
            {
                m_usb.context = nullptr;
                return false;
            }

            if (!libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG))
            {
                libusb_exit(m_usb.context);
                m_usb.context = nullptr;
                return false;
            }
            else
            {
                auto rc = libusb_hotplug_register_callback(
                    m_usb.context,
                    LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED,
                    LIBUSB_HOTPLUG_NO_FLAGS,
                    LIBUSB_HOTPLUG_MATCH_ANY,
                    LIBUSB_HOTPLUG_MATCH_ANY,
                    LIBUSB_HOTPLUG_MATCH_ANY,
                    hotplug_callback,
                    this,
                    NULL);

                if (rc != LIBUSB_SUCCESS)
                {
                    libusb_exit(m_usb.context);
                    m_usb.context = nullptr;
                    return false;
                }

                rc = libusb_hotplug_register_callback(
                    m_usb.context,
                    LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT,
                    LIBUSB_HOTPLUG_NO_FLAGS,
                    LIBUSB_HOTPLUG_MATCH_ANY,
                    LIBUSB_HOTPLUG_MATCH_ANY,
                    LIBUSB_HOTPLUG_MATCH_ANY,
                    hotplug_callback_detach,
                    this,
                    NULL);

                if (rc != LIBUSB_SUCCESS)
                {
                    libusb_exit(m_usb.context);
                    m_usb.context = nullptr;
                    return false;
                }
            }

            ListConnectedDevices();
        }

        return true;
    }

    std::vector<USBDevice> USBMonitor::ListConnectedDevices()
    {
        m_deviceMutex.lock();

        m_devices.clear();
        libusb_device **list;
        auto length = libusb_get_device_list(m_usb.context, &list);

        for (int i = 0; i < length; i++)
        {
            auto result = ToUSBDevice(list[i]);

            if (result.DidSucceed())
            {
                m_devices.push_back(result.GetValue());
            }
        }

        m_deviceMutex.unlock();
        return m_devices;
    }

    void USBMonitor::CheckForDeviceEvents() {
        if(m_usb.context != nullptr) {
            struct timeval timeout;
            timeout.tv_sec  = 0;
            timeout.tv_usec = 0;
            libusb_handle_events_timeout(m_usb.context, &timeout);
        }
    }
}