#ifndef KVM_USB_DEVICE_H
#define KVM_USB_DEVICE_H

#include <string>

namespace kvm {
  /**
   * Represents a device connected to the computer through USB. 
   */
  class USBDevice {
  public:

    typedef std::string VendorID;
    typedef std::string ProductID;

    /**
     * Initializing Constructor
     * @param vendorID Device Vendor ID
     * @param productID Device Product ID
     */
    USBDevice(const VendorID& vendorID, const ProductID& productID);

    /**
     * Get this device's vendor ID
     */
    const VendorID& GetVendorID() const;

    /**
     * Get this device's product ID
     */
    const ProductID& GetProductID() const;

    /**
     * Comparison Operator
     */
    bool operator==(const USBDevice& other) const;

  private:

    // Vendor ID
    VendorID m_vendorID;
    // Product ID
    ProductID m_productID;
  };
}

#endif // KVM_USB_DEVICE_H