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

    if (smcCallResult == kIOReturnSuccess && outputStruct.result == kSMCSuccess) {
        return outputStruct;
    } else if (smcCallResult == kIOReturnSuccess && outputStruct.result == kSMCKeyNotFound) {
        throw std::runtime_error("Given SMC-Key was not found!");
    } else if (smcCallResult == kIOReturnNotPrivileged) {
        throw std::runtime_error("Reading this key requires root privileges!");
    } else {
        throw std::runtime_error("An unknown error occurred while reading the SMC-Key!");
    }
}

void SMCKit::readKey(smc_key_t smcKey, SMCBytes &result) {
    SMCParamStruct inputStruct = SMCParamStruct();

    inputStruct.key = smcKey.code;
    inputStruct.keyInfo.dataSize = (UInt32) smcKey.info.size;
    inputStruct.data8 = kSMCReadKey;

    SMCParamStruct resultStruct = callSMC(inputStruct);
    std::copy(std::begin(resultStruct.bytes), std::end(resultStruct.bytes), std::begin(result));
}

void SMCKit::readKey(const std::string& keyCode, DataType typeInfo, SMCBytes& resultArray) {
    SMCKey key = SMCKey(keyCode, typeInfo);
    readKey(key, resultArray);
}

int SMCKit::getCPUTemp() {
    SMCBytes readResult = {0};
    readKey("TC0F", types.SP78, readResult);

    return (unsigned int) readResult[0];
}

int SMCKit::getFanCount() {
    SMCBytes readResult = {0};
    readKey("FNum", types.UInt8, readResult);

    return (unsigned int) readResult[0];
}

int SMCKit::getBatteryCount() {
    SMCBytes readResults = {0};
    readKey("BNum", types.UInt8, readResults);

    return (unsigned int) readResults[0];
}

bool SMCKit::isOnAC() {
    SMCBytes readResults = {0};
    readKey("BSIn", types.UInt8, readResults);

    return (bool) (readResults[0] & (unsigned)1);
}
