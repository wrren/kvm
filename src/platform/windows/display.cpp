#include <core/string.h>
#include <display/display.h>
#include <platform/types.h>
#include <iostream>
#include <WinUser.h>
#include <physicalmonitorenumerationapi.h>
#include <lowlevelmonitorconfigurationapi.h>

namespace kvm {
  BOOL MonitorEnumProc(HMONITOR monitor, HDC context, LPRECT clipping, LPARAM out) {
    Display::List* displays = reinterpret_cast<Display::List*>(out);

    MONITORINFOEX info;
    info.cbSize = sizeof(MONITORINFOEX);

    if(GetMonitorInfo(monitor, &info)) {
      std::string name(WideStringToString(static_cast<wchar_t*>(info.szDevice)));
      PlatformDisplay platform = { monitor };
      Display display(platform, displays->size(), name, Display::Input::UNKNOWN);
      displays->push_back(display);
    }


    return TRUE;
  }

  Display::List Display::ListDisplays() {
    Display::List displays;
    EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, reinterpret_cast<LPARAM>(&displays));
    return displays;
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