#include <display/display.h>
#include <CoreGraphics/CoreGraphics.h>
#include <IOKit/graphics/IOGraphicsLib.h>
#include <platform/mac/ddc.h>
#include <iostream>

#define MAX_DISPLAYS 64
#define MAX_DISPLAY_NAME_LENGTH 128
#define INPUT_SOURCE 0x60

namespace kvm {
    Display::List Display::ListDisplays() {
        Display::List list;

        CGDirectDisplayID ids[MAX_DISPLAYS];
        uint32_t returned;
        auto error = CGGetOnlineDisplayList(MAX_DISPLAYS, ids, &returned);

        if(error == 0) {
            for(int i = 0; i < returned; i++) {
                if(CGDisplayIsBuiltin(ids[i])) {
                    continue;
                }

                CFStringRef name;
                io_connect_t servicePort = DDC::IOServicePortFromCGDisplayID(ids[i], DDC::ServicePortType::IO_CONNECT);
                CFDictionaryRef info = IODisplayCreateInfoDictionary(servicePort, 0);
                IOObjectRelease(servicePort);

                CFDictionaryRef names = (CFDictionaryRef) CFDictionaryGetValue(info, CFSTR(kDisplayProductName));
                if(names && CFDictionaryGetValueIfPresent(names, CFSTR("en_US"), (const void**) &name)) {
                    char temp[MAX_DISPLAY_NAME_LENGTH];
                    CFStringGetCString(name, temp, MAX_DISPLAY_NAME_LENGTH, kCFStringEncodingASCII);
                    Display::Input input = Display::Input::UNKNOWN;
                    uint8_t currentValue;
                    if(DDC::GetControlValue(ids[i], INPUT_SOURCE, currentValue)) {
                        std::cout << "Input Value: " << currentValue << std::endl;
                        input = static_cast<Display::Input>(currentValue);
                    }

                    list.push_back(Display(ids[i], temp, input));
                }
                CFRelease(info);
            }
        }

        return list;
    }

    Display::Input Display::GetInput() {
        uint8_t input;
        if(DDC::GetControlValue(m_id, INPUT_SOURCE, input)) {
            m_input = static_cast<Display::Input>(input);
        } else {
            m_input = Display::Input::UNKNOWN;
        }
        return m_input;
    }

    bool Display::SetInput(Display::Input input) {
        if(input == m_input) {
            return true;
        }

        if(DDC::SetControlValue(m_id, INPUT_SOURCE, static_cast<uint8_t>(input))) {
            m_input = input;
            return true;
        }

        return false;
    }
}