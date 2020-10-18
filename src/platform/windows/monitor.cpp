#include <core/string.h>
#include <usb/monitor.h>
#include <SetupAPI.h>
#include <initguid.h>
#include <Usbiodef.h>
#include <devpkey.h>
#include <string>
#include <regex>
#include <sstream>
#include <iostream>
#include <strsafe.h>
#include <WinUser.h>
#include <Dbt.h>
#include <optional>
using namespace std::chrono_literals;
#define NAME_LENGTH_MAX 256

namespace kvm {
  typedef kvm::Result<std::string, DWORD> StrPropResult;

  StrPropResult GetDeviceStringProperty(HDEVINFO deviceInfo, PSP_DEVINFO_DATA deviceInfoData, PDEVPROPKEY key, PDEVPROPTYPE type) {
    DWORD       requiredSize = 0;
    LPTSTR      buffer = NULL;

    if(!SetupDiGetDeviceProperty(deviceInfo, deviceInfoData, key, type, NULL, 0, &requiredSize, 0) && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
      return StrPropResult(GetLastError());
    }

    if(*type != DEVPROP_TYPE_STRING) {
      return StrPropResult(ERROR_NOT_FOUND);
    }

    buffer = (LPTSTR) malloc(requiredSize);

    if(!SetupDiGetDeviceProperty(deviceInfo, deviceInfoData, key, type, (PBYTE) buffer, requiredSize, &requiredSize, 0)) {
      free(buffer);
      return StrPropResult(GetLastError());
    }

    std::string value = WideStringToString(buffer);
    free(buffer);
    return StrPropResult(value);
  }

  bool ExtractVendorAndProductID(const std::string& deviceID, USBDevice::VendorID& vendorID, USBDevice::ProductID& productID) {
    std::stringstream converter;
    std::regex        matcher(".*VID_([ABCDEF\\d]+)&PID_([ABCDEF\\d]+).*");
    std::smatch       matches;
    
    if(std::regex_match(deviceID, matches, matcher)) {
      converter << std::hex << matches[1];
      converter >> vendorID;
      converter.clear();

      converter << std::hex << matches[2];
      converter >> productID;
      converter.clear();

      return true;
    }

    return false;
  }

  std::optional<USBDevice> GetDeviceByVendorAndProductID(USBDevice::VendorID vendorID, USBDevice::ProductID productID) {
    BOOL                              success = TRUE;
    ULONG                             index = 0;
    DWORD                             requiredSize = 0;
    HDEVINFO                          deviceInfo = NULL;
    SP_DEVINFO_DATA                   deviceInfoData;
    SP_DEVICE_INTERFACE_DATA          deviceInterfaceData;
    DWORD                             lastError;
    DEVPROPKEY                        propertyKey   = DEVPKEY_Device_BusReportedDeviceDesc;
    DEVPROPTYPE                       propertyType  = 0;

    deviceInfo = SetupDiGetClassDevs((LPGUID) &GUID_CLASS_USB_DEVICE, NULL, NULL, (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE));

    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    for(index = 0; SetupDiEnumDeviceInfo(deviceInfo, index, &deviceInfoData); index++) {
      deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

      success = SetupDiEnumDeviceInterfaces(deviceInfo, 0, (LPGUID) &GUID_CLASS_USB_DEVICE, index, &deviceInterfaceData);

      if(!success) {
        continue;
      }

      propertyKey = DEVPKEY_Device_InstanceId;
      auto deviceInstanceID = GetDeviceStringProperty(deviceInfo, &deviceInfoData, &propertyKey, &propertyType);
      if(deviceInstanceID.DidFail()) {
        continue;
      }

      USBDevice::VendorID deviceVendorID;
      USBDevice::ProductID deviceProductID;

      if(ExtractVendorAndProductID(deviceInstanceID.GetValue(), deviceVendorID, deviceProductID) && vendorID == deviceVendorID && productID == deviceProductID) {
        propertyKey   = DEVPKEY_Device_BusReportedDeviceDesc;

        auto deviceName = GetDeviceStringProperty(deviceInfo, &deviceInfoData, &propertyKey, &propertyType);

        if(deviceName.DidFail()) {
          break;
        }   

        return std::optional<USBDevice>(USBDevice(deviceName.GetValue(), vendorID, productID));  
      }
    }

    return std::optional<USBDevice>();
  }

  LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
  {
    USBMonitor* monitor = (USBMonitor*) GetWindowLongPtr(hWnd, 0);
    if (uMsg == WM_DEVICECHANGE) {
      if(wParam == DBT_DEVICEREMOVECOMPLETE || wParam == DBT_DEVICEARRIVAL) {
        PDEV_BROADCAST_HDR pHdr = (PDEV_BROADCAST_HDR) lParam;
        switch( pHdr->dbch_devicetype ) {
          case DBT_DEVTYP_DEVICEINTERFACE:
            PDEV_BROADCAST_DEVICEINTERFACE pDevInf = (PDEV_BROADCAST_DEVICEINTERFACE) pHdr;
              std::string deviceID = WideStringToString(pDevInf->dbcc_name);
              USBDevice::VendorID vendorID;
              USBDevice::ProductID productID;

              if(ExtractVendorAndProductID(deviceID, vendorID, productID)) {
                auto result = GetDeviceByVendorAndProductID(vendorID, productID);

                if(result.has_value()) {
                  if(wParam == DBT_DEVICEARRIVAL) {
                    monitor->OnDeviceConnected(result.value());
                  } else if(wParam == DBT_DEVICEREMOVECOMPLETE) { 
                    monitor->OnDeviceDisconnected(result.value());
                  }
                }                
              }
            break;
        }
      }
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
  }

  USBMonitor::USBMonitor() {
    m_usb.messageWindow = NULL;
  }

  bool USBMonitor::Initialize() {
    if(m_usb.messageWindow == NULL) {
      WNDCLASS windowClass = {};
      windowClass.lpfnWndProc = WindowProcedure;
      LPCWSTR windowClassName = L"KVM Message-Only Window";
      windowClass.lpszClassName = windowClassName;
      windowClass.cbWndExtra = sizeof(USBMonitor*);
      if (!RegisterClass(&windowClass)) {
        return false;
      }
      m_usb.messageWindow = CreateWindow(windowClassName, 0, 0, 0, 0, 0, 0, HWND_MESSAGE, 0, 0, 0);

      SetWindowLongPtr(m_usb.messageWindow, 0, (LONG_PTR ) this);

      DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;

      ZeroMemory(&NotificationFilter, sizeof(NotificationFilter));
      NotificationFilter.dbcc_size        = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
      NotificationFilter.dbcc_devicetype  = DBT_DEVTYP_DEVICEINTERFACE;
      NotificationFilter.dbcc_classguid   = GUID_DEVINTERFACE_USB_DEVICE;

      if(!RegisterDeviceNotification( 
          m_usb.messageWindow,
          &NotificationFilter, 
          DEVICE_NOTIFY_WINDOW_HANDLE
      )) {
        return false;
      }
    }

    return m_usb.messageWindow != NULL;
  }

  std::vector<USBDevice> USBMonitor::ListConnectedDevices() {
    m_devices.clear();

    BOOL                              success = TRUE;
    ULONG                             index = 0;
    HDEVINFO                          deviceInfo = NULL;
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

      DEVPROPKEY  propertyKey   = DEVPKEY_Device_BusReportedDeviceDesc;
      DEVPROPTYPE propertyType  = 0;

      auto deviceName = GetDeviceStringProperty(deviceInfo, &deviceInfoData, &propertyKey, &propertyType);

      if(deviceName.DidFail()) {
        continue;
      }      

      propertyKey   = DEVPKEY_Device_InstanceId;

      auto deviceInstanceID = GetDeviceStringProperty(deviceInfo, &deviceInfoData, &propertyKey, &propertyType);
      if(deviceInstanceID.DidFail()) {
        continue;
      }   

      USBDevice::VendorID vendorID;
      USBDevice::ProductID productID;

      if(ExtractVendorAndProductID(deviceInstanceID.GetValue(), vendorID, productID)) {
        m_devices.push_back(USBDevice(deviceName.GetValue(), vendorID, productID));
      }
    }

    return m_devices;
  }


  void USBMonitor::CheckForDeviceEvents() {
    MSG msg;
    if (m_usb.messageWindow != NULL && PeekMessage(&msg, 0, 0, 0, PM_REMOVE) > 0) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
}