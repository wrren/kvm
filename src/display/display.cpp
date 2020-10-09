#include <display/display.h>
#include <algorithm>

namespace kvm {
    const uint8_t Display::InputVPCCode = 0x60;

    Display::Display(const PlatformDisplay& display, Display::Index index, const std::string& name) :
    m_display(display),
    m_index(index),
    m_name(name)
    {}

    const std::string& Display::GetName() const {
        return m_name;
    }

    std::string Display::GetInputAsString() const {
        return Display::InputToString(GetInput());
    }

    std::string Display::InputToString(Display::Input input) {
        switch(input) {
            case Display::Input::VGA1:      return "VGA 1";
            case Display::Input::VGA2:      return "VGA 2";
            case Display::Input::DVI1:      return "DVI 1";
            case Display::Input::DVI2:      return "DVI 2";
            case Display::Input::DP1:       return "DP 1";
            case Display::Input::DP2:       return "DP 2";
            case Display::Input::HDMI1:     return "HDMI 1";
            case Display::Input::HDMI2:     return "HDMI 2";
            default:
                return "Unknown";
        }
    }

    Display::Input Display::StringToInput(const std::string& string) {
        std::string lower = string;
        std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c){ 
            return std::tolower(c); 
        });

        if(lower == "vga1") {
            return Display::Input::VGA1; 
        }
        if(lower == "vga2") {
            return Display::Input::VGA2; 
        }
        if(lower == "dvi1") {
            return Display::Input::DVI1;
        }
        if(lower == "dvi2") {
            return Display::Input::DVI2; 
        }
        if(lower == "dp1") {
            return Display::Input::DP1;   
        }
        if(lower == "dp2") {
            return Display::Input::DP2;   
        }
        if(lower == "hdmi1") {
            return Display::Input::HDMI1; 
        }
        if(lower == "hdmi2") {
            return Display::Input::HDMI2;
        }

        return Display::Input::UNKNOWN;
    }

    const PlatformDisplay& Display::GetPlatformDisplay() const {
        return m_display;
    }
}