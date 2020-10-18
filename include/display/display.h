#ifndef KVM_DISPLAY_H
#define KVM_DISPLAY_H

#include <string>
#include <vector>
#include <map>
#include <ostream>
#include <platform/types.h>
#include <networking/serializable.h>

namespace kvm {
    class Display : public Serializable {
    public:
        typedef std::vector<Display>        List;
        typedef std::string                 ManufacturerID;
        typedef uint16_t                    ProductID;
        typedef uint32_t                    SerialNumber;

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
        typedef std::map<Display, Input> InputMap;

        /**
         * Default Constructor
         */
        Display();

        /**
         * Initialize a Display object with the given platform display structure, manufacturer ID, product ID, serial number and name.
         */
        Display(const PlatformDisplay& display, ManufacturerID manufacturer, ProductID product, SerialNumber serial, const std::string& name);

        /**
         * Get this display's manufacturer ID
         */
        const ManufacturerID& GetManufacturerID() const;

        /**
         * Get this display's product ID
         */
        ProductID GetProductID() const;

        /**
         * Get this display's serial number.
         */
        SerialNumber GetSerialNumber() const;

        /**
         * Get the name of this display.
         */
        const std::string& GetName() const;

        /**
         * Get the current input set for this display.
         */
        Input GetInput() const;

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

        /**
         * Serialize this display's identifying information.
         */
        virtual bool Serialize(NetworkBuffer& buffer) const override;

        /**
         * Deserialize this display's identifying information.
         */
        virtual bool Deserialize(NetworkBuffer& buffer) override;

        /**
         * Comparison Operators
         */
        bool operator==(const Display& other) const;
        bool operator!=(const Display& other) const;
        bool operator>=(const Display& other) const;
        bool operator<=(const Display& other) const;
        bool operator>(const Display& other) const;
        bool operator<(const Display& other) const;

        /**
         * Print the given display list to the specified output stream
         */
        static void PrintDisplayList(const List& displays, std::ostream& stream);

    private:

        /// Platform-specific Display Data
        PlatformDisplay m_display;
        /// Manufacturer ID
        ManufacturerID m_manufacturer;
        /// Product ID
        ProductID m_product;
        /// Display Serial Number
        SerialNumber m_serial;
        /// Display Name
        std::string m_name;
    };
}

#endif // KVM_DISPLAY_H