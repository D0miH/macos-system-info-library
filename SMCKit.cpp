#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/ps/IOPowerSources.h>
#include <IOKit/ps/IOPSKeys.h>

#include <stdexcept>
#include <string>

#include "SMCKit.h"

void SMCKit::open()
{
    // get the smc service
    io_service_t service = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceMatching("AppleSMC"));
    if (service == 0)
    {
        throw std::runtime_error("Unable to find the SMC service.");
    }

    kern_return_t openingResult = IOServiceOpen(service, mach_task_self_, 0, &connectionHandle);
    if (openingResult != kIOReturnSuccess)
    {
        throw std::runtime_error("Unable to connect to the SMC.");
    }

    IOObjectRelease(service);
}

void SMCKit::close()
{
    kern_return_t closeResult = IOServiceClose(connectionHandle);
    if (closeResult != kIOReturnSuccess)
    {
        throw std::runtime_error("Failed to close the connection to the SMC!");
    }
}

DataType SMCKit::getKeyInfo(std::string keyString)
{
    // create a new smc struct and set the key and data fields
    SMCParamStruct inputStruct = SMCParamStruct();
    inputStruct.key = Utils::stringToFourCharCode(keyString);
    inputStruct.data8 = kSMCGetKeyInfo;

    SMCParamStruct readResult = callSMC(inputStruct);

    return {readResult.keyInfo.dataType, readResult.keyInfo.dataSize};
}

SMCParamStruct SMCKit::callSMC(SMCParamStruct givenStruct,
                               SMCSelector smcSelector)
{
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

    if (smcCallResult == kIOReturnSuccess && outputStruct.result == kSMCSuccess)
    {
        return outputStruct;
    }
    else if (smcCallResult == kIOReturnSuccess && outputStruct.result == kSMCKeyNotFound)
    {
        throw std::runtime_error("Given SMC-Key was not found!");
    }
    else if (smcCallResult == kIOReturnNotPrivileged)
    {
        throw std::runtime_error("Reading this key requires root privileges!");
    }
    else
    {
        throw std::runtime_error("An unknown error occurred while reading the SMC-Key!");
    }
}

void SMCKit::readKey(smc_key_t smcKey, SMCBytes &result)
{
    SMCParamStruct inputStruct = SMCParamStruct();

    inputStruct.key = smcKey.code;
    inputStruct.keyInfo.dataSize = (UInt32)smcKey.info.size;
    inputStruct.data8 = kSMCReadKey;

    SMCParamStruct resultStruct = callSMC(inputStruct);
    std::copy(std::begin(resultStruct.bytes), std::end(resultStruct.bytes), std::begin(result));
}

void SMCKit::readKey(const std::string &keyCode, DataType typeInfo, SMCBytes &resultArray)
{
    SMCKey key = SMCKey(keyCode, typeInfo);
    readKey(key, resultArray);
}

int SMCKit::getCPUTemp()
{
    SMCBytes readResult = {0};
    readKey("TC0F", types.SP78, readResult);

    return (unsigned int)readResult[0];
}

int SMCKit::getFanCount()
{
    SMCBytes readResult = {0};
    readKey("FNum", types.UInt8, readResult);

    return (unsigned int)readResult[0];
}

int SMCKit::getMinFanSpeed(int fanID)
{
    SMCBytes readResult = {0};
    std::string keyString = "F" + std::to_string(fanID) + "Mn";

    try
    {
        // try to raed using the fpe2 type. Does not work for newer Macbooks.
        readKey(keyString, types.FPE2, readResult);
        UInt8 fpeValue[2] = {readResult[0], readResult[1]};
        return Utils::fpe2ToInt(fpeValue);
    }
    catch (const std::runtime_error &e)
    {
        // if reading the data using fpe2 fails try using flt type
        readKey(keyString, types.FLT, readResult);
        UInt8 fltValue[4] = {readResult[0], readResult[1], readResult[2], readResult[3]};
        return Utils::fltToInt(fltValue);
    }
}

int SMCKit::getMaxFanSpeed(int fanID)
{
    SMCBytes readResult = {0};
    std::string keyString = "F" + std::to_string(fanID) + "Mx";

    try
    {
        // try to raed using the fpe2 type. Does not work for newer Macbooks.
        readKey(keyString, types.FPE2, readResult);
        UInt8 fpeValue[2] = {readResult[0], readResult[1]};
        return Utils::fpe2ToInt(fpeValue);
    }
    catch (const std::runtime_error &e)
    {
        // if reading the data using fpe2 fails try using flt type
        readKey(keyString, types.FLT, readResult);
        UInt8 fltValue[4] = {readResult[0], readResult[1], readResult[2], readResult[3]};
        return Utils::fltToInt(fltValue);
    }
}

int SMCKit::getCurrentFanSpeed(int fanID)
{
    SMCBytes readResult = {0};
    std::string keyString = "F" + std::to_string(fanID) + "Ac";

    try
    {
        // try to raed using the fpe2 type. Does not work for newer Macbooks.
        readKey(keyString, types.FPE2, readResult);
        UInt8 fpeValue[2] = {readResult[0], readResult[1]};
        return Utils::fpe2ToInt(fpeValue);
    }
    catch (const std::runtime_error &e)
    {
        // if reading the data using fpe2 fails try using flt type
        readKey(keyString, types.FLT, readResult);
        UInt8 fltValue[4] = {readResult[0], readResult[1], readResult[2], readResult[3]};
        return Utils::fltToInt(fltValue);
    }
}

int SMCKit::getBatteryCount()
{
    SMCBytes readResults = {0};
    readKey("BNum", types.UInt8, readResults);

    return (unsigned int)readResults[0];
}

bool SMCKit::isChargingBattery()
{
    SMCBytes readResults = {0};
    readKey("BSIn", types.UInt8, readResults);

    return (bool)(readResults[0] & (unsigned)1);
}

float SMCKit::getBatteryHealth()
{
    // get the
    io_registry_entry_t batteryRegistry = IORegistryEntryFromPath(kIOMasterPortDefault, "IOService:/AppleACPIPlatformExpert/SMB0/AppleECSMBusController/AppleSmartBatteryManager/AppleSmartBattery");
    CFMutableDictionaryRef psDict;
    if (IORegistryEntryCreateCFProperties(batteryRegistry, &psDict, kCFAllocatorDefault, 0))
    {
        throw std::runtime_error("Something went wrong while reading the battery health.");
    }

    // get the design capacity
    SInt32 designCapacity;
    CFNumberGetValue((CFNumberRef)CFDictionaryGetValue(psDict, CFSTR(kIOPSDesignCapacityKey)), (CFNumberType)3, &designCapacity);
    // get the current max capacity
    SInt32 maxCapacity;
    CFNumberGetValue((CFNumberRef)CFDictionaryGetValue(psDict, CFSTR("MaxCapacity")), (CFNumberType)3, &maxCapacity);

    return (float)maxCapacity / (float)designCapacity;
}