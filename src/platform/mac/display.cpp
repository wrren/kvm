#include <display/display.h>
#include <display/edid.h>
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
        if(error == 0) {
            for(int i = 0; i < returned; i++) {
                if(CGDisplayIsBuiltin(ids[i])) {
                    continue;
                }
                PlatformDisplay display{id: ids[i]};
                EDID edid;
                Display::ManufacturerID     manufacturer;
                Display::ProductID          product;
                Display::SerialNumber       serial;
                std::string                 name;
                
                if(DDC::ReadEDID(display, edid) && edid.GetManufacturerID(manufacturer) && edid.GetProductID(product) && edid.GetSerialNumber(serial) && edid.GetDisplayName(name)) {
                    list.push_back(Display(display, manufacturer, product, serial, name));
                }
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