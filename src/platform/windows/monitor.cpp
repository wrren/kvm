#include <core/string.h>
#include <usb/monitor.h>
#include <SetupAPI.h>
#include <initguid.h>
#include <Usbiodef.h>
#include <devpkey.h>
#include <string>
#include <regex>
#include <sstream>

#define NAME_LENGTH_MAX 256

namespace kvm {
  BOOL GetDeviceProperty(HDEVINFO DeviceInfoSet, PSP_DEVINFO_DATA DeviceInfoData, DWORD Property, LPTSTR* ppBuffer) {
    BOOL bResult;
    DWORD requiredLength = 0;
    DWORD lastError;

    if (ppBuffer == NULL)
    {
        return FALSE;
    }

    *ppBuffer = NULL;

    bResult = SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                               DeviceInfoData,
                                               Property ,
                                               NULL,
                                               NULL,
                                               0,
                                               &requiredLength);
    lastError = GetLastError();

    if ((requiredLength == 0) || (bResult != FALSE && lastError != ERROR_INSUFFICIENT_BUFFER))
    {
        return FALSE;
    }

    *ppBuffer = (LPTSTR) malloc(requiredLength);

    if (*ppBuffer == NULL)
    {
        return FALSE;
    }

    bResult = SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                                DeviceInfoData,
                                                Property ,
                                                NULL,
                                                (PBYTE) *ppBuffer,
                                                requiredLength,
                                                &requiredLength);
    if(bResult == FALSE)
    {
        free(*ppBuffer);
        *ppBuffer = NULL;
        return FALSE;
    }

    return TRUE;
  }

  USBMonitor::USBMonitor() {

  }

  bool USBMonitor::Initialize() {
    return true;
  }

  std::vector<USBDevice> USBMonitor::ListConnectedDevices() {
    std::vector<USBDevice> devices;

    std::stringstream                 converter;
    std::string                       description;
    BOOL                              success = TRUE;
    ULONG                             index = 0;
    DWORD                             requiredSize = 0;
    HDEVINFO                          deviceInfo = NULL;
    PSP_DEVICE_INTERFACE_DETAIL_DATA  deviceDetailData = NULL;
    SP_DEVINFO_DATA                   deviceInfoData;
    SP_DEVICE_INTERFACE_DATA          deviceInterfaceData;

    deviceInfo = SetupDiGetClassDevs((LPGUID) &GUID_CLASS_USB_DEVICE, NULL, NULL, (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE));

    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    for(index = 0; SetupDiEnumDeviceInfo(deviceInfo, index, &deviceInfoData); index++) {
      deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

      success = SetupDiEnumDeviceInterfaces(deviceInfo, 0, (LPGUID) &GUID_CLASS_USB_DEVICE, index, &deviceInterfaceData);

      if(!success) {
        continue;
      }

      success = SetupDiGetDeviceInterfaceDetail(deviceInfo, &deviceInterfaceData, NULL, 0, &requiredSize, NULL);

      if (!success && GetLastError() != ERROR_INSUFFICIENT_BUFFER)
      {
          continue;
      }

      deviceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA) malloc(requiredSize);
      if(deviceDetailData == NULL) {
        continue;
      }

      deviceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

      success = SetupDiGetDeviceInterfaceDetail(deviceInfo, &deviceInterfaceData, deviceDetailData, requiredSize, &requiredSize, NULL);

      if(!success) {
        free(deviceDetailData);
        continue;
      }

      free(deviceDetailData);

      PTSTR deviceDescription;

      success = GetDeviceProperty(deviceInfo, &deviceInfoData, SPDRP_DEVICEDESC, &deviceDescription);
      if(!success) {
        continue;
      }

      description = WideStringToString(deviceDescription);

      free(deviceDescription);

      PTSTR deviceInstanceId;

      success = SetupDiGetDeviceInstanceId(deviceInfo, &deviceInfoData, NULL, 0, &requiredSize);

      if(!success && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        continue;
      }

      DEVPROPKEY propertyKey    = DEVPKEY_NAME;
      DEVPROPTYPE propertyType  = DEVPROP_TYPE_STRING;

      success = SetupDiGetDeviceInterfaceProperty(deviceInfo, &deviceInterfaceData, &propertyKey, &propertyType, NULL, 0, &requiredSize, 0);

      if(!success && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        continue;
      }

      LPTSTR deviceName = (LPTSTR) malloc(requiredSize + 1);

      success = SetupDiGetDeviceInterfaceProperty(deviceInfo, &deviceInterfaceData, &propertyKey, &propertyType, (PBYTE) deviceName, requiredSize, &requiredSize, 0);

      if(!success && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        free(deviceName);
        continue;
      }

      description = WideStringToString(deviceName);

      free(deviceName);

      deviceInstanceId = (LPTSTR) malloc(requiredSize + 1);

      success = SetupDiGetDeviceInstanceId(deviceInfo, &deviceInfoData, deviceInstanceId, requiredSize, &requiredSize);

      if(!success) {
        continue;
      }

      std::string deviceInstanceIdString(WideStringToString(deviceInstanceId));
      std::regex matcher("USB\\\\VID_([ABCDEF\\d]+)&PID_([ABCDEF\\d]+).+");
      std::smatch matches;
      
      if(std::regex_match(deviceInstanceIdString, matches, matcher)) {
        USBDevice::VendorID vendorID;
        USBDevice::ProductID productID;

        converter << std::hex << matches[1];
        converter >> vendorID;
        converter.clear();

        converter << std::hex << matches[2];
        converter >> productID;
        converter.clear();

        devices.push_back(USBDevice(description, vendorID, productID));
      }

      free(deviceInstanceId);
    }


    return devices;
  }


  void USBMonitor::CheckForDeviceEvents() {
    
  }
}