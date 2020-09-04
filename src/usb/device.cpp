#include <usb/device.h>

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
}