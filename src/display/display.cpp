#include <display/display.h>

namespace kvm {
    Display::Display(DisplayID id, const std::string& name, Display::Input input) :
    m_id(id),
    m_name(name),
    m_input(input)
    {}

    const std::string& Display::GetName() const {
        return m_name;
    }

    std::string Display::GetInputAsString() const {
        return Display::InputToString(m_input);
    }

    std::string Display::InputToString(Display::Input input) {
        switch(input) {
            case Display::Input::VGA1:      return "VGA 1";
            case Display::Input::VGA2:      return "VGA 2";
            case Display::Input::DVI1:      return "DVI 1";
            case Display::Input::DVI2:      return "DVI 2";
            case Display::Input::DP1:       return "DP 1";
            case Display::Input::DP2:       return "DP 2";
            default:
                return "Unknown";
        }
    }

    DisplayID Display::GetID() const {
        return m_id;
    }
}