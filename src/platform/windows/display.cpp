#include <core/string.h>
#include <display/display.h>
#include <display/edid.h>
#include <platform/types.h>
#include <iostream>
#include <initguid.h>
#include <WinUser.h>
#include <physicalmonitorenumerationapi.h>
#include <lowlevelmonitorconfigurationapi.h>
#include <SetupAPI.h>
#include <tchar.h>
#include <Ntddvdeo.h>

#define NAME_SIZE 128

namespace kvm {
  bool ReadEDID(HKEY deviceRegistryKey, EDID& edid) {
    DWORD dwType, requiredSize = NAME_SIZE;
    TCHAR valueName[NAME_SIZE];
 
    BYTE edidData[1024];
    DWORD edidSize = sizeof(edidData);
 
    for (LONG i = 0, retValue = ERROR_SUCCESS; retValue != ERROR_NO_MORE_ITEMS; ++i) {
      if(RegEnumValue(deviceRegistryKey, i, &valueName[0], &requiredSize, NULL, &dwType, edidData, &edidSize) != ERROR_SUCCESS || wcscmp(valueName, L"EDID") != 0) {
        continue;
      }
 
      edid.SetData(edidData, edidSize);

      return true;
    }

    return false;
  }

  BOOL MonitorEnumProc(HMONITOR monitor, HDC context, LPRECT clipping, LPARAM out) {
    Display::List* displays = reinterpret_cast<Display::List*>(out);

    MONITORINFOEX info;
    info.cbSize = sizeof(MONITORINFOEX);

    if(GetMonitorInfo(monitor, &info)) {
      DISPLAY_DEVICE device;
      EDID edid;
      device.cb = sizeof(DISPLAY_DEVICE);
      if(EnumDisplayDevices(info.szDevice, 0, &device, EDD_GET_DEVICE_INTERFACE_NAME)) {
        std::string name(WideStringToString(static_cast<wchar_t*>(device.DeviceID)));
        PlatformDisplay platform = { monitor, device };

        HDEVINFO                          deviceInfoSet;
        HKEY                              deviceRegistryKey;
        SP_DEVINFO_DATA                   deviceInfoData;
        SP_DEVICE_INTERFACE_DATA          deviceInterfaceData;
        PSP_DEVICE_INTERFACE_DETAIL_DATA  deviceInterfaceDetailData;
        ULONG                             index = 0;
        Display::ManufacturerID           manufacturer;
        Display::ProductID                product;
        Display::SerialNumber             serial;
        DWORD                             requiredSize = 0;

        deviceInfoSet = SetupDiGetClassDevs(&GUID_DEVINTERFACE_MONITOR, NULL, NULL, DIGCF_DEVICEINTERFACE);

        if(deviceInfoSet != NULL) {
          for(index = 0; GetLastError() != ERROR_NO_MORE_ITEMS; index++) {
            memset(&deviceInterfaceData, 0, sizeof(SP_DEVICE_INTERFACE_DATA));
            deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

            if(SetupDiEnumDeviceInterfaces(deviceInfoSet, NULL, &GUID_DEVINTERFACE_MONITOR, index, &deviceInterfaceData)) { 
              if(!SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, NULL, 0, &requiredSize, NULL) && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                deviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA) malloc(requiredSize);
                memset(deviceInterfaceDetailData, 0, sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA));
                deviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
                if(SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, deviceInterfaceDetailData, requiredSize, &requiredSize, NULL)) {
                  if(_wcsicmp(&(deviceInterfaceDetailData->DevicePath[0]), device.DeviceID) == 0) {
                    memset(&deviceInfoData, 0, sizeof(SP_DEVINFO_DATA));
                    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

                    if(SetupDiEnumDeviceInfo(deviceInfoSet, index, &deviceInfoData)) {
                      deviceRegistryKey = SetupDiOpenDevRegKey(deviceInfoSet, &deviceInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);

                      if(deviceRegistryKey && ReadEDID(deviceRegistryKey, edid)) {
                        if(edid.GetProductID(product) && edid.GetSerialNumber(serial) && edid.GetManufacturerID(manufacturer)) {
                          edid.GetDisplayName(name);
                          Display display(platform, manufacturer, product, serial, name);
                          displays->push_back(display);
                        }                        
                      }
                    }
                  }
                }
                free(deviceInterfaceDetailData);
              }
            }            
          }
        }        
      }
    }

    return TRUE;
  }

  Display::List Display::ListDisplays() {
    Display::List displays;
    EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, reinterpret_cast<LPARAM>(&displays));

    

    return displays;
  }

  Display::Input Display::GetInput() const {
    DWORD monitorCount;
    Display::Input input = Display::Input::UNKNOWN;

    if(GetNumberOfPhysicalMonitorsFromHMONITOR(m_display.handle, &monitorCount) == FALSE) {
      return input;
    }

    if(monitorCount == 0) {
      return input;
    }

    bool result = false;
    LPPHYSICAL_MONITOR monitors = (LPPHYSICAL_MONITOR) malloc(monitorCount * sizeof(PHYSICAL_MONITOR));

    if(GetPhysicalMonitorsFromHMONITOR(m_display.handle, monitorCount, monitors)) {
      for(int i = 0; i < monitorCount; i++) {
        PHYSICAL_MONITOR monitor = monitors[i];
        DWORD capabilityValue;

        if(GetVCPFeatureAndVCPFeatureReply(monitor.hPhysicalMonitor, Display::InputVPCCode, NULL, &capabilityValue, NULL)) {
          input = static_cast<Display::Input>(capabilityValue);
        }
      }
      
      DestroyPhysicalMonitors(monitorCount, monitors);
    }
    
    free(monitors);

    return input;
  }

  bool Display::SetInput(Display::Input input) {
    DWORD monitorCount;

    if(GetNumberOfPhysicalMonitorsFromHMONITOR(m_display.handle, &monitorCount) == FALSE) {
      return false;
    }

    if(monitorCount == 0) {
      return false;
    }

    bool result = false;
    LPPHYSICAL_MONITOR monitors = (LPPHYSICAL_MONITOR) malloc(monitorCount & sizeof(PHYSICAL_MONITOR));

    if(GetPhysicalMonitorsFromHMONITOR(m_display.handle, monitorCount, monitors)) {
      for(int i = 0; i < monitorCount; i++) {
        PHYSICAL_MONITOR monitor = monitors[i];
        DWORD capabilitiesStringLength;

        if(GetCapabilitiesStringLength(monitor.hPhysicalMonitor, &capabilitiesStringLength)) {
          LPSTR capabilitiesString = (LPSTR) malloc(capabilitiesStringLength);

          if(CapabilitiesRequestAndCapabilitiesReply(monitor.hPhysicalMonitor, capabilitiesString, capabilitiesStringLength)) {
            result = SetVCPFeature(monitor.hPhysicalMonitor, Display::InputVPCCode, static_cast<uint8_t>(input));
          }

          free(capabilitiesString);
        }
      }
      
      DestroyPhysicalMonitors(monitorCount, monitors);
    }
    
    free(monitors);

    return result;
  }
}