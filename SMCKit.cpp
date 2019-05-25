#include <IOKit/IOKitLib.h>
#include <stdexcept>

#include "SMCKit.h"

void SMCKit::open() {
    // get the smc service
    io_service_t service = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceMatching("AppleSMC"));
    if(service == 0) {
        throw std::runtime_error("Unable to find the smc service.");
    }

    kern_return_t openingResult = IOServiceOpen(service, mach_task_self_, 0, &connectionHandle);
    if(openingResult != kIOReturnSuccess) {
        throw std::runtime_error("Unable to connect to the smc.");
    }

    IOObjectRelease(service);
}

void SMCKit::close() {
    kern_return_t closeResult = IOServiceClose(connectionHandle);
    if(closeResult != kIOReturnSuccess) {
        throw std::runtime_error("Failed to close the connection to the smc.");
    }
}


