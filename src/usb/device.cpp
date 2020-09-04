#include <usb/device.h>

namespace kvm {

  USBDevice::USBDevice(const USBDevice::VendorID& vendorID, const USBDevice::ProductID& productID) :
  m_vendorID(vendorID),
  m_productID(productID)
  {}

  const USBDevice::VendorID& USBDevice::GetVendorID() const {
    return m_vendorID;
  }

  const USBDevice::ProductID& USBDevice::GetProductID() const {
    return m_productID;
  }

  bool USBDevice::operator==(const USBDevice& other) const {
    return m_vendorID == other.m_vendorID && m_productID == other.m_productID;
  }
}