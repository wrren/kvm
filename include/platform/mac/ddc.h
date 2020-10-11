#ifndef KVM_PLATFORM_DDC_MAC_H
#define KVM_PLATFORM_DDC_MAC_H

#include <display/display.h>
#include <display/edid.h>
#include <CoreGraphics/CoreGraphics.h>
#include <IOKit/graphics/IOGraphicsLib.h>

namespace kvm {
    /**
     * Allows DDC commands to be sent to a Display.
     */
    class DDC {
    public:

        typedef struct {
            uint8_t controlID;
            uint8_t maxValue;
            uint8_t currentValue;
            bool    success;
        } ReadCommand;

        typedef struct {
            uint8_t controlID;
            uint8_t newValue;
        } WriteCommand;

        enum class ServicePortType {
            IO_CONNECT,
            FRAMEBUFFER
        };

        /**
         * Get the IO Service Port that corresponds with the given display ID and port type.
         */
        static io_service_t IOServicePortFromPlatformDisplay(const PlatformDisplay& display, ServicePortType type);

        /**
         * Send a read command to the given display.
         */
        static bool Read(const PlatformDisplay& display, ReadCommand& command);

        /**
         * Send a write command to the given display.
         */
        static bool Write(const PlatformDisplay& display, const WriteCommand& command);

        /**
         * Get the current value for the given control.
         */
        static bool GetControlValue(const PlatformDisplay& display, uint8_t controlID, uint8_t& currentValue);

        /**
         * Set a new value for the given control.
         */
        static bool SetControlValue(const PlatformDisplay& display, uint8_t controlID, uint8_t newValue);

        /**
         * Attempt to read the given display's EDID data
         */
        static bool ReadEDID(const PlatformDisplay& display, EDID& edid);
    };
}

#endif // KVM_PLATFORM_DDC_MAC_H