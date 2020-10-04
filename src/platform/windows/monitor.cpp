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
using namespace std::chrono_literals;
#define NAME_LENGTH_MAX 256

namespace kvm {
  typedef kvm::Result<std::string, DWORD> StrPropResult;
  HWND messageWindow = NULL;

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

  LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
  {
    USBMonitor* monitor = (USBMonitor*) GetWindowLongPtr(hWnd, 0);
    if (uMsg == WM_DEVICECHANGE) {
      if(wParam == DBT_DEVICEREMOVECOMPLETE || wParam == DBT_DEVICEARRIVAL) {
        PDEV_BROADCAST_HDR pHdr = (PDEV_BROADCAST_HDR) lParam;
        switch( pHdr->dbch_devicetype ) {
          case DBT_DEVTYP_DEVICEINTERFACE:
            std::cout << "Device Changed" << std::endl;
            PDEV_BROADCAST_DEVICEINTERFACE pDevInf = (PDEV_BROADCAST_DEVICEINTERFACE) pHdr;
              std::string deviceID = WideStringToString(pDevInf->dbcc_name);
              USBDevice::VendorID vendorID;
              USBDevice::ProductID productID;

              if(ExtractVendorAndProductID(deviceID, vendorID, productID)) {
                USBDevice device("", vendorID, productID);
                if(wParam == DBT_DEVICEARRIVAL) {
                  std::cout << "Device Connected" << std::endl;
                  monitor->OnDeviceConnected(device);
                } else if(wParam == DBT_DEVICEREMOVECOMPLETE) { 
                  std::cout << "Device Disconnected" << std::endl;
                  monitor->OnDeviceDisconnected(device);
                }
              }
            break;
        }
      }
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
  }

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

  USBMonitor::USBMonitor() {

  }

  bool USBMonitor::Initialize() {
    if(messageWindow == NULL) {
      WNDCLASS windowClass = {};
      windowClass.lpfnWndProc = WindowProcedure;
      LPCWSTR windowClassName = L"KVM Message-Only Window";
      windowClass.lpszClassName = windowClassName;
      windowClass.cbWndExtra = sizeof(USBMonitor*);
      if (!RegisterClass(&windowClass)) {
        std::cout << "Failed to register window class" << std::endl;
        return 1;
      }
      messageWindow = CreateWindow(windowClassName, 0, 0, 0, 0, 0, 0, HWND_MESSAGE, 0, 0, 0);

      SetWindowLongPtr(messageWindow, 0, (LONG_PTR ) this);

      DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;

      ZeroMemory( &NotificationFilter, sizeof(NotificationFilter) );
      NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
      NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
      NotificationFilter.dbcc_classguid = GUID_DEVINTERFACE_USB_DEVICE;

      if(!RegisterDeviceNotification( 
          messageWindow,         // events recipient
          &NotificationFilter,              // type of device
          DEVICE_NOTIFY_WINDOW_HANDLE       // type of recipient handle
      )) {
        return false;
      }
    }
    

    return messageWindow != NULL;
  }

  std::vector<USBDevice> USBMonitor::ListConnectedDevices() {
    m_devices.clear();

    BOOL                              success = TRUE;
    ULONG                             index = 0;
    DWORD                             requiredSize = 0;
    HDEVINFO                          deviceInfo = NULL;
    SP_DEVINFO_DATA                   deviceInfoData;
    SP_DEVICE_INTERFACE_DATA          deviceInterfaceData;
    DWORD                             lastError;

    deviceInfo = SetupDiGetClassDevs((LPGUID) &GUID_CLASS_USB_DEVICE, NULL, NULL, (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE));

    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    for(index = 0; SetupDiEnumDeviceInfo(deviceInfo, index, &deviceInfoData); index++) {
      deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

      success = SetupDiEnumDeviceInterfaces(deviceInfo, 0, (LPGUID) &GUID_CLASS_USB_DEVICE, index, &deviceInterfaceData);

      if(!success) {
        continue;
      }

      PTSTR deviceInstanceId;

      success = SetupDiGetDeviceInstanceId(deviceInfo, &deviceInfoData, NULL, 0, &requiredSize);

      if(!success && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        continue;
      }

      DEVPROPKEY  propertyKey   = DEVPKEY_Device_BusReportedDeviceDesc;
      DEVPROPTYPE propertyType  = 0;

      auto deviceName = GetDeviceStringProperty(deviceInfo, &deviceInfoData, &propertyKey, &propertyType);

      if(deviceName.DidFail()) {
        std::cout << "Get Device Property Failed: " << deviceName.GetError() << std::endl;
        continue;
      }      

      propertyKey   = DEVPKEY_Device_InstanceId;

      auto deviceInstanceID = GetDeviceStringProperty(deviceInfo, &deviceInfoData, &propertyKey, &propertyType);
      if(deviceInstanceID.DidFail()) {
        std::cout << "Get Device Property Failed: " << deviceInstanceID.GetError() << std::endl;
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
    std::this_thread::sleep_for(1s);
    MSG msg;
    if (messageWindow != NULL && GetMessage(&msg, 0, 0, 0) > 0) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
}