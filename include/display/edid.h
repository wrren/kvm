#ifndef KVM_EDID_H
#define KVM_EDID_H

#include <display/display.h>
#include <string>

namespace kvm {
  class EDID {
  public:

    /**
     * Default Constructor
     */
    EDID();

    /**
     * Initializing Constructor
     */
    EDID(uint8_t* buffer, size_t bufferSize);

    /**
     * Set the EDID data that's parsed by this object.
     */
    void SetData(uint8_t* buffer, size_t bufferSize);

    /**
     * Clear the underlying EDID data buffer.
     */
    void ResetData();

    /**
     * Attempt to read the display manufacturer ID
     */
    bool GetManufacturerID(Display::ManufacturerID& manufacturer) const;

    /**
     * Attempt to read the display product ID.
     */
    bool GetProductID(Display::ProductID& productID) const;

    /**
     * Attempt to read the display serial number.
     */
    bool GetSerialNumber(Display::SerialNumber& serial) const;

    /**
     * Attempt to read the display name.
     */
    bool GetDisplayName(std::string& name) const;

    /**
     * Default Destructor
     */
    ~EDID();

  private:

    /// EDID Data Buffer
    uint8_t* m_buffer;
    /// Data Buffer Size
    size_t m_bufferSize;
  };
}

#endif // KVM_EDID_H