#ifndef KVM_DISPLAY_H
#define KVM_DISPLAY_H

#include <string>
#include <vector>
#include <platform/types.h>

namespace kvm {
    class Display {
    public:
        typedef uint8_t                     Index;
        typedef std::vector<Display>        List;

        static const uint8_t InputVPCCode;

        enum class Input : uint8_t {
            UNKNOWN = 0,
            VGA1    = 1,
            VGA2    = 2,
            DVI1    = 3,
            DVI2    = 4,
            DP1     = 15,
            DP2     = 16,
            HDMI1   = 17,
            HDMI2   = 18
        };

        /**
         * Initialize a Display object with the given ID, name and current input.
         */
        Display(const PlatformDisplay& display, Index index, const std::string& name, Input input = Input::UNKNOWN);

        /**
         * Get this display's index.
         */
        Index GetIndex() const;

        /**
         * Get the name of this display.
         */
        const std::string& GetName() const;

        /**
         * Get the current input set for this display.
         */
        Input GetInput();

        /**
         * Get the current input as a string.
         */
        std::string GetInputAsString() const;

        /**
         * Set the current input for this display.
         */
        bool SetInput(Input input);

        /**
         * Get the Platform Display identifier
         */
        const PlatformDisplay& GetPlatformDisplay() const;

        /**
         * List connected displays.
         */
        static List ListDisplays();

        /**
         * Convert an input value to human-readable form.
         */
        static std::string InputToString(Input input);

        /**
         * Convert a string value to a valid Input value. Returns UNKNOWN
         * on failure.
         */
        static Input StringToInput(const std::string& string);

    private:

        /// Display ID
        PlatformDisplay m_display;
        /// Display Index
        Index m_index;
        /// Display Name
        std::string m_name;
        /// Current Input
        Input m_input;
    };
}

#endif // KVM_DISPLAY_H