#include <display/edid.h>
#include <platform/functions.h>
#include <cstring>

namespace kvm {
  EDID::EDID() :
  m_buffer(nullptr),
  m_bufferSize(0)
  {}

  EDID::EDID(uint8_t* buffer, size_t bufferSize) :
  EDID() {
    SetData(buffer, bufferSize);
  }

  void EDID::SetData(uint8_t* buffer, size_t bufferSize) {
    ResetData();

    if(buffer != nullptr && bufferSize > 0) {
      m_buffer = new uint8_t[bufferSize];
      memcpy(m_buffer, buffer, bufferSize);
      m_bufferSize = bufferSize;
    }
  }

  void EDID::ResetData() {
    if(m_buffer != nullptr) {
      delete[] m_buffer;
      m_bufferSize = 0;
    }
  }

  bool EDID::GetManufacturerID(Display::ManufacturerID& manufacturer) const {
    if(m_bufferSize >= 10) {
      uint16_t data;
      char characters[3];
      memcpy(&data, &m_buffer[8], sizeof(uint16_t));
      data = NetworkToHost(data);

      characters[0] = 64 + ((data >> 10) & 0x1F);
      characters[1] = 64 + ((data >> 5) & 0x1F);
      characters[2] = 64 + (data & 0x1F);

      manufacturer.assign(&characters[0], 3);

      return true;
    }
    return false;
  }

  bool EDID::GetProductID(Display::ProductID& product) const {
    if(m_bufferSize >= 10 + sizeof(Display::ProductID)) {
      memcpy(&product, &m_buffer[10], sizeof(Display::ProductID));
      return true;
    }
    return false;
  }

  bool EDID::GetSerialNumber(Display::SerialNumber& serial) const {
    if(m_bufferSize >= 12 + sizeof(Display::SerialNumber)) {
      memcpy(&serial, &m_buffer[12], sizeof(Display::SerialNumber));
      return true;
    }
    return false;
  }

  bool EDID::GetDisplayName(std::string& name) const {
    if(m_bufferSize >= 126) {
      char nameArray[14];
      memset(nameArray, 0, 14);
      for(int offset = 72; offset < 126; offset += 18) {
        const uint8_t type = m_buffer[offset + 3];

        if(m_buffer[offset + 3] == 0xFC) {
          int arrayOffset = 0;

          for(int charOffset = offset + 5; charOffset < offset + 18; charOffset++, arrayOffset++) {
            if(m_buffer[charOffset] == '\n') {
              break;
            }
            nameArray[arrayOffset] = m_buffer[charOffset];
          }
          name = nameArray;
          return true;
        }
      }
    }
    return false;
  }

  EDID::~EDID() {
    ResetData();
  }
}