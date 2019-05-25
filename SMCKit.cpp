#include <IOKit/IOKitLib.h>
#include <stdexcept>

#include "SMCKit.h"

void SMCKit::open() {
    // get the smc service
    io_service_t service = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceMatching("AppleSMC"));
    if (service == 0) {
        throw std::runtime_error("Unable to find the SMC service.");
    }

    kern_return_t openingResult = IOServiceOpen(service, mach_task_self_, 0, &connectionHandle);
    if (openingResult != kIOReturnSuccess) {
        throw std::runtime_error("Unable to connect to the SMC.");
    }

    IOObjectRelease(service);
}

void SMCKit::close() {
    kern_return_t closeResult = IOServiceClose(connectionHandle);
    if (closeResult != kIOReturnSuccess) {
        throw std::runtime_error("Failed to close the connection to the SMC!");
    }
}

SMCParamStruct SMCKit::callSMC(SMCParamStruct givenStruct,
                     SMCSelector smcSelector = kSMCHandleYPCEvent) {
    // create an output struct to save the result
    SMCParamStruct outputStruct = SMCParamStruct();
    size_t outputStructSize = sizeof(outputStruct);
    size_t inputStructSize = sizeof(givenStruct);

    // make the call to the SMC
    kern_return_t smcCallResult = IOConnectCallStructMethod(connectionHandle,
                                                            smcSelector,
                                                            &givenStruct,
                                                            inputStructSize,
                                                            &outputStruct,
                                                            &outputStructSize);

    if(smcCallResult == kIOReturnSuccess && outputStruct.result == kSMCSuccess) {
        return outputStruct;
    } else if(smcCallResult == kIOReturnSuccess && outputStruct.result == kSMCKeyNotFound) {
        throw std::runtime_error("Given SMC-Key was not found!");
    } else if(smcCallResult == kIOReturnNotPrivileged) {
        throw std::runtime_error("Reading this key requires root privileges!");
    } else {
        throw std::runtime_error("An unknown error occurred while reading the SMC-Key!");
    }
}
