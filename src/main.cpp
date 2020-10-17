#include <iostream>
#include <iomanip>
#include <kvm.h>
#include <usb/monitor.h>
#include <display/display.h>
#include <vector>
#include <string>

enum class RunMode {
  WATCH,
  LIST_DEVICES
};

const uint16_t DefaultPort = 10191;

typedef struct {
  RunMode                   mode;
  uint16_t                  port;
  kvm::USBDevice::VendorID  vendor;
  kvm::USBDevice::ProductID product;
  kvm::Display::InputMap    inputs;
  std::vector<std::string>  nodes;
} Options;

bool ParseOptions(int argc, char** argv, Options& options) {
  bool sawVendor = false, sawProduct = false;
  options.inputs.clear();
  
  auto displays = kvm::Display::ListDisplays();
  for(auto display : displays) {
    options.inputs[display] = display.GetInput();
  }
  options.port = DefaultPort;
  options.mode = RunMode::WATCH;

  for(int i = 1; i != argc; i++) {
    if(strcmp(argv[i], "--list-devices") == 0) {
      options.mode = RunMode::LIST_DEVICES;
      return true;
    }
    if(strcmp(argv[i], "--port") == 0 && (i + 1) < argc) {
      options.port = atoi(argv[++i]);
    } else if(strcmp(argv[i], "--vendor") == 0 && (i + 1) < argc) {
      options.vendor = atoi(argv[++i]);
      sawVendor = true;
    } else if(strcmp(argv[i], "--product") == 0 && (i + 1) < argc) {
      options.product = atoi(argv[++i]);
      sawProduct = true;
    } else if(strcmp(argv[i], "--preferred-input") == 0 && (i + 2) < argc) {
      bool foundDisplay = false;
      auto serial       = atoi(argv[++i]);
      auto input        = kvm::Display::StringToInput(argv[++i]);

      for(auto display : displays) {
        if(display.GetSerialNumber() == serial) {
          options.inputs[display] = input;
          foundDisplay = true;
        }
      }

      if(!foundDisplay) {
        std::cerr << "Failed to find display with serial number " << serial << std::endl;
        return false;
      }
    } else if(strcmp(argv[i], "--node") == 0 && (i + 1) < argc) {
      options.nodes.push_back((argv[++i]));
    }
  }

  if(!sawVendor) {
    std::cerr << "Please specify a USB device Vendor ID to be watched for connection events." << std::endl;
    return false;
  }

  if(!sawProduct) {
    std::cerr << "Please specify a USB device Product ID to be watched for connection events." << std::endl;
    return false;
  }

  return true;
}

class ConsoleListener : public kvm::KVM::Listener {
public:

  virtual void OnNodeConnected(const kvm::Node& node) override {
    std::cout << "Node Connected: " << kvm::AddressToString(node.GetAddress()) << std::endl;
  }

  virtual void OnNodeDisconnected(const kvm::Node& node) override {
    std::cout << "Node Disconnected: " << kvm::AddressToString(node.GetAddress()) << std::endl;
  }

  virtual void OnStateChange(kvm::KVM::State previousState, kvm::KVM::State newState) override {
    switch(newState) {
      case kvm::KVM::State::ACTIVE:
        std::cout << "All Inputs Set Correctly, Now Active" << std::endl;
        break;

      case kvm::KVM::State::INACTIVE:
        std::cout << "Trigger Device Lost, Now Inactive" << std::endl;
        break;

      case kvm::KVM::State::REQUESTING_INPUT:
        std::cout << "Trigger Device Connected, Requesting Input Changes" << std::endl;
        break;
    }
  }

  virtual void OnTriggerDeviceConnected(const kvm::USBDevice& device) override {
    std::cout << "Trigger Device Connected: " << device.GetDescription() << std::endl;
  }

  virtual void OnDisplayInputChangesRequested(const kvm::Display::InputMap& changes) override {
    std::cout << "Requested " << changes.size() << " Input Changes..." << std::endl;
  }
};

int main(int argc, char** argv) {
  Options options;

  if(ParseOptions(argc, argv, options)) {
    kvm::KVM kvm(options.port);
    if(!kvm.Initialize()) {
      std::cout << "Failed to initialize USB monitor." << std::endl;
      return EXIT_FAILURE;
    }

    if(options.mode == RunMode::LIST_DEVICES) {
      kvm::USBDevice::PrintDeviceList(kvm.ListUSBDevices(), std::cout);
      std::cout << std::endl;
      kvm::Display::PrintDisplayList(kvm::Display::ListDisplays(), std::cout);
    } else {
      ConsoleListener listener;
      kvm.AddListener(&listener);
      kvm::USBDevice trigger(options.vendor, options.product);

      kvm.SetTriggerDevice(trigger);
      kvm.SetDesiredInputs(options.inputs);

      std::cout << "Listening on Port " << options.port << std::endl;
      std::cout << "Watching for USB Device " << options.vendor << ":" << options.product << std::endl;
      
      for(auto input : options.inputs) {
        std::cout << "Prefer Input " << kvm::Display::InputToString(input.second) << " For Display " << input.first.GetName() << " (" << input.first.GetSerialNumber() << ")" << std::endl;
      }

      while(true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        kvm.Pump();
      }
    }
  }  

  return EXIT_SUCCESS;
}
