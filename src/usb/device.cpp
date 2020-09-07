#include <usb/device.h>
#include <iostream>

namespace kvm {

  USBDevice::USBDevice(const USBDevice::Description& description, USBDevice::VendorID vendorID, const USBDevice::ProductID productID) :
  m_description(description),
  m_vendorID(vendorID),
  m_productID(productID)
  {}

  const USBDevice::Description& USBDevice::GetDescription() const {
    return m_description;
  }

  USBDevice::VendorID USBDevice::GetVendorID() const {
    return m_vendorID;
  }

  USBDevice::ProductID USBDevice::GetProductID() const {
    return m_productID;
  }

  bool USBDevice::operator==(const USBDevice& other) const {
    return m_vendorID == other.m_vendorID && m_productID == other.m_productID;
  }

  void USBDevice::PrintDeviceList(const std::vector<USBDevice>& devices, std::ostream& stream) {
    stream.setf(std::ios::left, std::ios::adjustfield);
    stream.width(80);
    stream << "Device Name";
    stream.width(20);
    stream << "Vendor ID";
    stream.width(20);
    stream << "Product ID";
    stream << std::endl;

    stream.width(120);
    stream.fill('=');
    stream << "=" << std::endl;
    stream.fill(' ');

    for(auto device : devices) {
      stream.setf(std::ios::left, std::ios::adjustfield);
      stream.width(80);
      stream << device.GetDescription();
      stream.width(20);
      stream << device.GetVendorID();
      stream.width(20);
      stream << device.GetProductID();
      stream << std::endl;
    }
  }
}