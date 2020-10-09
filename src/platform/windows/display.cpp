#include <core/string.h>
#include <display/display.h>
#include <platform/types.h>
#include <iostream>
#include <WinUser.h>
#include <physicalmonitorenumerationapi.h>
#include <lowlevelmonitorconfigurationapi.h>
#include <SetupAPI.h>

const GUID GUID_CLASS_MONITOR = {0x4d36e96e, 0xe325, 0x11ce, 0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18};

namespace kvm {
  BOOL MonitorEnumProc(HMONITOR monitor, HDC context, LPRECT clipping, LPARAM out) {
    Display::List* displays = reinterpret_cast<Display::List*>(out);

    MONITORINFOEX info;
    info.cbSize = sizeof(MONITORINFOEX);

    if(GetMonitorInfo(monitor, &info)) {
      DISPLAY_DEVICE device;
      device.cb = sizeof(DISPLAY_DEVICE);
      if(EnumDisplayDevices(info.szDevice, 0, &device, 0)) {
        std::string name(WideStringToString(static_cast<wchar_t*>(device.DeviceID)));
        PlatformDisplay platform = { monitor, device };
        Display display(platform, displays->size(), name);
        displays->push_back(display);
      }
      
    }

    return TRUE;
  }

  Display::List Display::ListDisplays() {
    Display::List displays;
    EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, reinterpret_cast<LPARAM>(&displays));

    HDEVINFO          deviceInfoSet;
    HKEY              deviceRegistryKey;
    SP_DEVINFO_DATA   deviceInfoData;
    ULONG             index = 0;
    DWORD             uniqueID[123];

    deviceInfoSet = SetupDiGetClassDevsEx(&GUID_CLASS_MONITOR, NULL, NULL, DIGCF_PRESENT, NULL, NULL, NULL);

    if(deviceInfoSet != NULL) {
      for(index = 0; GetLastError() != ERROR_NO_MORE_ITEMS; index++) {
        memset(deviceInfoData, 0, sizeof(SP_DEVICE_INFO_DATA));
        deviceInfoData.cbSize = sizeof(SP_DEVICE_INFO_DATA);

        if(SetupDiEnumDeviceInfo(deviceInfoSet, index, &deviceInfoData)) {
          DWORD uniqueID;
          if(SetupDiGetDeviceRegistryProperty(deviceInfoSet, &deviceInfoData, SPDRP_DEVICEDESC, NULL, (PBYTE) &uniqueID, sizeof(uniqueID), NULL)) {
            deviceRegistryKey = SetupDiOpenDevRegKey(deviceInfoSet, &deviceInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_ALL_ACCESS);

            if(deviceRegistryKey) {
              unsigned char edidData[1024];
              LONG          result = ERROR_SUCCESS;
              ULONG         keyIndex = 0, edidIndex = 0;
              DWORD         type, requiredSize, edidSize = sizeof(edidData);

              for(keyIndex = 0; result != ERROR_NO_MORE_ITEMS; keyIndex++) {
                char valueName[128];
                char hex[3];

                result = RegEnumValue(deviceRegistryKey, keyIndex, &valueName[0], &requiredSize, NULL, &type, edidData, &edidIndex);

                if(result == ERROR_SUCCESS) {
                  if(strcmp(valueName, "EDID") == 0) {
                    
                  }
                }
              }

            }
          }
        }
      }
    }

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