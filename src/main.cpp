#include <iostream>
#include <iomanip>
#include <usb/monitor.h>
#include <display/display.h>

int main(int argc, char** argv) {
  kvm::USBMonitor monitor;

  if(auto error = monitor.Initialize()) {
    std::cout << error.value() << std::endl;
    return EXIT_FAILURE;
  }

  auto devices = monitor.ListConnectedDevices();
  
  kvm::USBDevice::PrintDeviceList(devices, std::cout);

  auto displays = kvm::Display::ListDisplays();

  std::cout << "Displays:" << std::endl;
  for(auto display : displays) {
    std::cout << display.GetName() << ", Input: " << display.GetInputAsString() << std::endl;
  }

  displays[0].SetInput(kvm::Display::Input::HDMI1);

  while(true) {
    monitor.CheckForDeviceEvents();
  }

  return EXIT_SUCCESS;
}
