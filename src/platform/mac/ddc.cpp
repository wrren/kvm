#include <platform/mac/ddc.h>
#include <IOKit/IOKitLib.h>
extern "C" {
    #include <IOKit/i2c/IOI2CInterface.h>
}
#include <ApplicationServices/ApplicationServices.h>
#define MIN_REPLY_DELAY 30000000

#ifndef kMaxRequests
#define kMaxRequests 10
#endif

#ifndef _IOKIT_IOFRAMEBUFFER_H
#define kIOFBDependentIDKey	"IOFBDependentID"
#define kIOFBDependentIndexKey	"IOFBDependentIndex"
#endif

namespace kvm {
    dispatch_semaphore_t GetDisplayQueue(const PlatformDisplay& display) {
        static UInt64 queueCount = 0;
        static struct DDCQueue {CGDirectDisplayID id; dispatch_semaphore_t queue;} *queues = NULL;
        dispatch_semaphore_t queue = NULL;
        if (!queues)
            queues = (DDCQueue*) calloc(50, sizeof(*queues));
        UInt64 i = 0;
        while (i < queueCount)
            if (queues[i].id == display.id)
                break;
            else
                i++;
        if (queues[i].id == display.id)
            queue = queues[i].queue;
        else
            queues[queueCount++] = (struct DDCQueue){display.id, (queue = dispatch_semaphore_create(1))};
        return queue;
    }

    io_service_t DDC::IOServicePortFromPlatformDisplay(const PlatformDisplay& display, DDC::ServicePortType type) {
        io_iterator_t iterator;
        io_service_t servicePort, match = 0;

        const char* serviceName;

        if(type == DDC::ServicePortType::IO_CONNECT) {
            serviceName = "IODisplayConnect";
        } else {
            serviceName = IOFRAMEBUFFER_CONFORMSTO;
        }

        kern_return_t error = IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceMatching(serviceName), &iterator);

        if(error) {
            return 0;
        }

        while((servicePort = IOIteratorNext(iterator)) != MACH_PORT_NULL) {
            CFNumberRef vendorIDRef;
            CFNumberRef productIDRef;
            CFNumberRef serialNumberRef;

            CFDictionaryRef displayInfo = IODisplayCreateInfoDictionary(servicePort, kIODisplayOnlyPreferredName);
            Boolean success =   CFDictionaryGetValueIfPresent(displayInfo, CFSTR(kDisplayVendorID),  (const void**) &vendorIDRef) &&
                                CFDictionaryGetValueIfPresent(displayInfo, CFSTR(kDisplayProductID), (const void**) &productIDRef);

            if(!success) {
                CFRelease(displayInfo);
                continue;
            }

            SInt32 vendorID;
            CFNumberGetValue(vendorIDRef, kCFNumberSInt32Type, &vendorID);
            SInt32 productID;
            CFNumberGetValue(productIDRef, kCFNumberSInt32Type, &productID);

            SInt32 serialNumber = 0;
            if (CFDictionaryGetValueIfPresent(displayInfo, CFSTR(kDisplaySerialNumber), (const void**) & serialNumberRef))
            {
                CFNumberGetValue(serialNumberRef, kCFNumberSInt32Type, &serialNumber);
            }

            if( CGDisplayVendorNumber(display.id) != vendorID ||
                CGDisplayModelNumber(display.id)  != productID ||
                CGDisplaySerialNumber(display.id) != serialNumber )
            {
                CFRelease(displayInfo);
                IOObjectRelease(servicePort);
                continue;
            }

            CFRelease(displayInfo);
            match = servicePort;
            break;
        }

        IOObjectRelease(iterator);

        return match;
    }

    bool SendRequestToDisplay(IOI2CRequest* request, const PlatformDisplay& display) {
        dispatch_semaphore_t queue = GetDisplayQueue(display);
        dispatch_semaphore_wait(queue, DISPATCH_TIME_FOREVER);
        bool result = false;
        io_service_t framebuffer; 

        if ((framebuffer = DDC::IOServicePortFromPlatformDisplay(display, DDC::ServicePortType::FRAMEBUFFER))) {
            IOItemCount busCount;
            if (IOFBGetI2CInterfaceCount(framebuffer, &busCount) == KERN_SUCCESS) {
                IOOptionBits bus = 0;
                while (bus < busCount) {
                    io_service_t interface;
                    if (IOFBCopyI2CInterfaceForBus(framebuffer, bus++, &interface) != KERN_SUCCESS)
                        continue;

                    IOI2CConnectRef connect;
                    if (IOI2CInterfaceOpen(interface, kNilOptions, &connect) == KERN_SUCCESS) {
                        result = (IOI2CSendRequest(connect, kNilOptions, request) == KERN_SUCCESS);
                        IOI2CInterfaceClose(connect, kNilOptions);
                    }
                    IOObjectRelease(interface);
                    if (result) break;
                }
            }
            IOObjectRelease(framebuffer);
        }
        if (request->replyTransactionType == kIOI2CNoTransactionType)
            usleep(20000);
        dispatch_semaphore_signal(queue);
        return result && request->result == KERN_SUCCESS;
    }

    UInt32 SupportedTransactionType() {
        /*
            With my setup (Intel HD4600 via displaylink to 'DELL U2515H') the original app failed to read ddc and freezes my system.
            This happens because AppleIntelFramebuffer do not support kIOI2CDDCciReplyTransactionType.
            So this version comes with a reworked ddc read function to detect the correct TransactionType.
            --SamanVDR 2016
        */
        kern_return_t   kr;
        io_iterator_t   io_objects;
        io_service_t    io_service;

        kr = IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceNameMatching("IOFramebufferI2CInterface"), &io_objects);

        if (kr != KERN_SUCCESS) {
            printf("E: Fatal - No matching service! \n");
            return 0;
        }

        UInt32 supportedType = 0;

        while((io_service = IOIteratorNext(io_objects)) != MACH_PORT_NULL)
        {
            CFMutableDictionaryRef service_properties;
            CFIndex types = 0;
            CFNumberRef typesRef;

            kr = IORegistryEntryCreateCFProperties(io_service, &service_properties, kCFAllocatorDefault, kNilOptions);
            if (kr == KERN_SUCCESS)
            {
                if (CFDictionaryGetValueIfPresent(service_properties, CFSTR(kIOI2CTransactionTypesKey), (const void**)&typesRef))
                    CFNumberGetValue(typesRef, kCFNumberCFIndexType, &types);

                /*
                We want DDCciReply but Simple is better than No-thing.
                Combined and DisplayPortNative are not useful in our case.
                */
                if (types) {
                    // kIOI2CSimpleTransactionType = 1
                    if ( 0 != ((1 << kIOI2CSimpleTransactionType) & (UInt64)types)) {
                        supportedType = kIOI2CSimpleTransactionType;
                    }

                    // kIOI2CDDCciReplyTransactionType = 2
                    if ( 0 != ((1 << kIOI2CDDCciReplyTransactionType) & (UInt64)types)) {
                        supportedType = kIOI2CDDCciReplyTransactionType;
                    }
                } else printf("E: Fatal - No supported Transaction Types! \n");

                CFRelease(service_properties);
            }

            IOObjectRelease(io_service);

            // Mac OS offers three framebuffer devices, but we can leave here
            if (supportedType > 0) return supportedType;
        }

        return supportedType;
    }

    bool DDC::Write(const PlatformDisplay& display, const DDC::WriteCommand& command) {
        IOI2CRequest request;
        bzero(&request, sizeof(request));
        uint8_t payload[128];

        request.commFlags           = 0;
        request.sendAddress         = 0x6E;
        request.sendTransactionType = kIOI2CSimpleTransactionType;
        request.sendBuffer          = (vm_address_t) &payload[0];
        request.sendBytes           = 7;

        payload[0] = 0x51;
        payload[1] = 0x84;
        payload[2] = 0x03;
        payload[3] = command.controlID;
        payload[4] = command.newValue >> 8;
        payload[5] = command.newValue & 255;
        payload[6] = 0x6E ^ payload[0] ^ payload[1] ^ payload[2] ^ payload[3] ^ payload[4] ^ payload[5];

        request.replyTransactionType            = kIOI2CNoTransactionType;
        request.replyBytes                      = 0;

        return SendRequestToDisplay(&request, display);
    }

    bool DDC::Read(const PlatformDisplay& display, DDC::ReadCommand& command) {
        IOI2CRequest request;
        UInt8 reply_data[11] = {};
        bool result = false;
        UInt8 data[128];

        for (int i=1; i<= kMaxRequests; i++) {
            bzero(&request, sizeof(request));

            request.commFlags                       = 0;
            request.sendAddress                     = 0x6E;
            request.sendTransactionType             = kIOI2CSimpleTransactionType;
            request.sendBuffer                      = (vm_address_t) &data[0];
            request.sendBytes                       = 5;
            // Certain displays / graphics cards require a long-enough delay to give a response.
            // Relying on retry will not help if the delay is too short.
            request.minReplyDelay                   = MIN_REPLY_DELAY * kNanosecondScale;
            // FIXME: this should be tuneable at runtime
            // https://github.com/kfix/ddcctl/issues/57
            // incorrect values for GPU-vendor can cause kernel panic
            // https://developer.apple.com/documentation/iokit/ioi2crequest/1410394-minreplydelay?language=objc

            data[0] = 0x51;
            data[1] = 0x82;
            data[2] = 0x01;
            data[3] = command.controlID;
            data[4] = 0x6E ^ data[0] ^ data[1] ^ data[2] ^ data[3];
    #ifdef TT_SIMPLE
            request.replyTransactionType    = kIOI2CSimpleTransactionType;
    #elif defined TT_DDC
            request.replyTransactionType    = kIOI2CDDCciReplyTransactionType;
    #else
            request.replyTransactionType    = SupportedTransactionType();
    #endif
            request.replyAddress            = 0x6F;
            request.replySubAddress         = 0x51;

            request.replyBuffer = (vm_address_t) reply_data;
            request.replyBytes = sizeof(reply_data);

            result = SendRequestToDisplay(&request, display);
            result = (result && reply_data[0] == request.sendAddress && reply_data[2] == 0x2 && reply_data[4] == command.controlID && reply_data[10] == (request.replyAddress ^ request.replySubAddress ^ reply_data[1] ^ reply_data[2] ^ reply_data[3] ^ reply_data[4] ^ reply_data[5] ^ reply_data[6] ^ reply_data[7] ^ reply_data[8] ^ reply_data[9]));

            if (result) { // checksum is ok
                if (i > 1) {
                    printf("D: Tries required to get data: %d \n", i);
                }
                break;
            }

            if (request.result == kIOReturnUnsupportedMode)
                printf("E: Unsupported Transaction Type! \n");

            // reset values and return 0, if data reading fails
            if (i >= kMaxRequests) {
                command.success = false;
                command.maxValue = 0;
                command.currentValue = 0;
                printf("E: No data after %d tries! \n", i);
                return false;
            }

            usleep(40000); // 40msec -> See DDC/CI Vesa Standard - 4.4.1 Communication Error Recovery
        }
        command.success = true;
        command.maxValue = reply_data[7];
        command.currentValue = reply_data[9];

        return true;
    }

    bool DDC::GetControlValue(const PlatformDisplay& display, uint8_t controlID, uint8_t& currentValue) {
        ReadCommand command;
        command.controlID       = controlID;
        command.maxValue        = 0;
        command.currentValue    = 0;

        if(!DDC::Read(display, command)) {
            return false;
        }
        currentValue = command.currentValue;
        return true;
    }

    bool DDC::SetControlValue(const PlatformDisplay& display, uint8_t controlID, uint8_t newValue) {
        WriteCommand command;
        command.controlID       = controlID;
        command.newValue        = newValue;

        return DDC::Write(display, command);
    }

    bool DDC::ReadEDID(const PlatformDisplay& display, EDID& edid) {
        IOI2CRequest request = {};
        UInt8 data[128] = {};
        request.sendAddress = 0xA0;
        request.sendTransactionType = kIOI2CSimpleTransactionType;
        request.sendBuffer = (vm_address_t) data;
        request.sendBytes = 0x01;
        data[0] = 0x00;
        request.replyAddress = 0xA1;
        request.replyTransactionType = kIOI2CSimpleTransactionType;
        request.replyBuffer = (vm_address_t) data;
        request.replyBytes = sizeof(data);
        if(!SendRequestToDisplay(&request, display)) {
            return false;
        }
        UInt32 i = 0;
        UInt8 sum = 0;
        while (i < request.replyBytes) {
            if (i % 128 == 0) {
                if (sum) break;
                sum = 0;
            }
            sum += data[i++];
        }
        
        if(!sum) {
            edid.SetData(data, 128);
        }

        return !sum;
    }
}