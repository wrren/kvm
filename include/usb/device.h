#ifndef KVM_USB_DEVICE_H
#define KVM_USB_DEVICE_H

#include <string>
#include <iostream>
#include <vector>

namespace kvm {
  /**
   * Represents a device connected to the computer through USB. 
   */
  class USBDevice {
  public:

    typedef std::string Description;
    typedef uint16_t VendorID;
    typedef uint16_t ProductID;

    /**
     * Initializing Constructor
     * @param description Device description
     * @param vendorID Device Vendor ID
     * @param productID Device Product ID
     */
    USBDevice(const Description& description, const VendorID vendorID, const ProductID productID);

    /**
     * Get the human-readable description of this device
     */
    const Description& GetDescription() const;

    /**
     * Get this device's vendor ID
     */
    VendorID GetVendorID() const;

    /**
     * Get this device's product ID
     */
    ProductID GetProductID() const;

    /**
     * Comparison Operator
     */
    bool operator==(const USBDevice& other) const;

    /**
     * Prints a table detailing the given list of USB devices to the given output stream.
     */
    static void PrintDeviceList(const std::vector<USBDevice>& devices, std::ostream& stream);

  private:
    
    // Human-readable device description
    Description m_description;
    // Vendor ID
    VendorID m_vendorID;
    // Product ID
    ProductID m_productID;
  };
}

#endif // KVM_USB_DEVICE_H