#include <display/display.h>
#include <CoreGraphics/CoreGraphics.h>
#include <IOKit/graphics/IOGraphicsLib.h>
#include <platform/mac/ddc.h>
#include <iostream>

#define MAX_DISPLAYS 64
#define MAX_DISPLAY_NAME_LENGTH 128

namespace kvm {
    Display::List Display::ListDisplays() {
        Display::List list;

        CGDirectDisplayID ids[MAX_DISPLAYS];
        uint32_t returned;
        auto error = CGGetOnlineDisplayList(MAX_DISPLAYS, ids, &returned);
        Display::Index index = 0;
        if(error == 0) {
            for(int i = 0; i < returned; i++) {
                if(CGDisplayIsBuiltin(ids[i])) {
                    continue;
                }
                PlatformDisplay display{id: ids[i]};

                CFStringRef name;
                io_connect_t servicePort = DDC::IOServicePortFromPlatformDisplay(display, DDC::ServicePortType::IO_CONNECT);
                CFDictionaryRef info = IODisplayCreateInfoDictionary(servicePort, 0);
                IOObjectRelease(servicePort);

                CFDictionaryRef names = (CFDictionaryRef) CFDictionaryGetValue(info, CFSTR(kDisplayProductName));
                if(names && CFDictionaryGetValueIfPresent(names, CFSTR("en_US"), (const void**) &name)) {
                    char temp[MAX_DISPLAY_NAME_LENGTH];
                    CFStringGetCString(name, temp, MAX_DISPLAY_NAME_LENGTH, kCFStringEncodingASCII);
                    list.push_back(Display(display, index, temp));
                    index++;
                }
                CFRelease(info);
            }
        }

        return list;
    }

    Display::Input Display::GetInput() const {
        uint8_t input;
        if(DDC::GetControlValue(m_display, Display::InputVPCCode, input)) {
            return static_cast<Display::Input>(input);
        } else {
            return Display::Input::UNKNOWN;
        }
    }

    bool Display::SetInput(Display::Input input) {
        if(input == GetInput()) {
            return true;
        }

        if(DDC::SetControlValue(m_display, Display::InputVPCCode, static_cast<uint8_t>(input))) {
            return true;
        }

        return false;
    }
}