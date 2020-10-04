#include <iostream>
#include <iomanip>
#include <usb/monitor.h>
#include <display/display.h>

enum class RunMode {
  WATCH,
  LIST_DEVICES
};

typedef struct {
  RunMode                   mode;
  kvm::USBDevice::VendorID  vendor;
  kvm::USBDevice::ProductID product;
  kvm::Display::Input       input;
} KVMOptions;

bool ParseOptions(int argc, char** argv, KVMOptions& options) {


  return false;
}

int main(int argc, char** argv) {
  kvm::USBMonitor monitor;

  if(!monitor.Initialize()) {
    std::cout << "Failed to initialize USB monitor." << std::endl;
    return EXIT_FAILURE;
  }

  auto devices = monitor.ListConnectedDevices();
  
  kvm::USBDevice::PrintDeviceList(devices, std::cout);

  auto displays = kvm::Display::ListDisplays();

  std::cout << "Displays:" << std::endl;
  for(auto display : displays) {
    std::cout << display.GetName() << ", Input: " << display.GetInputAsString() << std::endl;
  }

  while(true) {
    monitor.CheckForDeviceEvents();
  }

  return EXIT_SUCCESS;
}
