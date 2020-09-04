#include <iostream>
#include <iomanip>
#include <usb/monitor.h>

void print_device_list(const std::vector<kvm::USBDevice>& devices) {
  std::cout.setf(std::ios::left, std::ios::adjustfield);
  std::cout.width(80);
  std::cout << "Device Name";
  std::cout.width(10);
  std::cout << "Vendor ID";
  std::cout.width(10);
  std::cout << "Product ID";
  std::cout << std::endl;

  std::cout.width(100);
  std::cout.fill('=');
  std::cout << "=" << std::endl;
  std::cout.fill(' ');

  for(auto device: devices) {
    std::cout.setf(std::ios::left, std::ios::adjustfield);
    std::cout.width(80);
    std::cout << device.GetDescription();
    std::cout.width(10);
    std::cout << device.GetVendorID();
    std::cout.width(10);
    std::cout << device.GetProductID();
    std::cout << std::endl;
  }
}

int main(int argc, char** argv) {
  kvm::USBMonitor monitor;

  if(auto error = monitor.Initialize()) {
    std::cout << error.value() << std::endl;
    return EXIT_FAILURE;
  }

  auto devices = monitor.ListConnectedDevices();
  print_device_list(devices);

  while(true) {
    monitor.CheckForDeviceEvents();
  }

  return EXIT_SUCCESS;
}
