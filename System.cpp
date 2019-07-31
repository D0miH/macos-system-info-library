#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/ps/IOPowerSources.h>
#include <IOKit/ps/IOPSKeys.h>

#include <stdexcept>
#include <string>

#include "System.h"

void System::open()
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

void System::close()
{
    kern_return_t closeResult = IOServiceClose(connectionHandle);
    if (closeResult != kIOReturnSuccess)
    {
        throw std::runtime_error("Failed to close the connection to the SMC!");
    }
}

DataType System::getKeyInfo(std::string keyString)
{
    // create a new smc struct and set the key and data fields
    SMCParamStruct inputStruct = SMCParamStruct();
    inputStruct.key = Utils::stringToFourCharCode(keyString);
    inputStruct.data8 = kSMCGetKeyInfo;

    SMCParamStruct readResult = callSMC(inputStruct);

    return {readResult.keyInfo.dataType, readResult.keyInfo.dataSize};
}

SMCParamStruct System::callSMC(SMCParamStruct givenStruct,
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

void System::readKey(smc_key_t smcKey, SMCBytes &result)
{
    SMCParamStruct inputStruct = SMCParamStruct();

    inputStruct.key = smcKey.code;
    inputStruct.keyInfo.dataSize = (UInt32)smcKey.info.size;
    inputStruct.data8 = kSMCReadKey;

    SMCParamStruct resultStruct = callSMC(inputStruct);
    std::copy(std::begin(resultStruct.bytes), std::end(resultStruct.bytes), std::begin(result));
}

void System::readKey(const std::string &keyCode, DataType typeInfo, SMCBytes &resultArray)
{
    SMCKey key = SMCKey(keyCode, typeInfo);
    readKey(key, resultArray);
}

int System::getCpuTemp()
{
    SMCBytes readResult = {0};
    readKey("TC0F", types.SP78, readResult);

    return (unsigned int)readResult[0];
}

std::vector<float> System::getMemoryUsage()
{
    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    mach_port_t host = mach_host_self();
    vm_statistics64_data_t stats;

    kern_return_t result = host_statistics64(host, HOST_VM_INFO64, (host_info64_t)&stats, &count);

    if (result != KERN_SUCCESS)
    {
        throw std::runtime_error("An error occured while getting the memory usage.");
    }

    vm_size_t page_size = vm_kernel_page_size;
    // divide by 1_073_741_824 to get gigabyte
    float freeMem = (double)(stats.free_count * page_size) / (double)1073741824;
    float activeMem = (double)(stats.active_count * page_size) / (double)1073741824;
    float inactiveMem = (double)(stats.inactive_count * page_size) / (double)1073741824;
    float wiredMem = (double)(stats.wire_count * page_size) / (double)1073741824;
    float compressedMem = (double)(stats.compressor_page_count * page_size) / (double)1073741824;

    std::vector<float> resultVec;
    resultVec.push_back(freeMem);
    resultVec.push_back(activeMem);
    resultVec.push_back(inactiveMem);
    resultVec.push_back(wiredMem);
    resultVec.push_back(compressedMem);

    return resultVec;
}

float System::getTotalMemory()
{
    mach_msg_type_number_t count = HOST_BASIC_INFO_COUNT;
    mach_port_t host = mach_host_self();
    host_basic_info_data_t info;

    kern_return_t result = host_info(host, HOST_BASIC_INFO, (host_info_t)&info, &count);

    if (result != KERN_SUCCESS)
    {
        throw std::runtime_error("An error occured while getting the total memory of the machine.");
    }

    return (double)info.max_mem / (double)1073741824;
}

std::vector<float> System::getCpuUsage()
{
    cpu_tick_t curCpuTicks = getCpuLoadInfo();

    float userDiff = curCpuTicks.userTicks - prevCpuTicks.userTicks;
    float sysDiff = curCpuTicks.systemTicks - prevCpuTicks.systemTicks;
    float idleDiff = curCpuTicks.idleTicks - prevCpuTicks.idleTicks;
    float niceDiff = curCpuTicks.niceTicks - prevCpuTicks.niceTicks;

    float curTotalTicks = userDiff + sysDiff + idleDiff + niceDiff;

    std::vector<float> resultVec;
    resultVec.push_back(userDiff / curTotalTicks * 100);
    resultVec.push_back(sysDiff / curTotalTicks * 100);
    resultVec.push_back(idleDiff / curTotalTicks * 100);
    resultVec.push_back(niceDiff / curTotalTicks * 100);

    prevCpuTicks = curCpuTicks;

    return resultVec;
}

int System::getFanCount()
{
    SMCBytes readResult = {0};
    readKey("FNum", types.UInt8, readResult);

    return (unsigned int)readResult[0];
}

int System::getMinFanSpeed(int fanID)
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

int System::getMaxFanSpeed(int fanID)
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

int System::getCurrentFanSpeed(int fanID)
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

int System::getBatteryCount()
{
    SMCBytes readResults = {0};
    readKey("BNum", types.UInt8, readResults);

    return (unsigned int)readResults[0];
}

bool System::isChargingBattery()
{
    SMCBytes readResults = {0};
    readKey("BSIn", types.UInt8, readResults);

    return (bool)(readResults[0] & (unsigned)1);
}

float System::getBatteryHealth()
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

int System::getBatteryCycles()
{
    // get the
    io_registry_entry_t batteryRegistry = IORegistryEntryFromPath(kIOMasterPortDefault, "IOService:/AppleACPIPlatformExpert/SMB0/AppleECSMBusController/AppleSmartBatteryManager/AppleSmartBattery");
    CFMutableDictionaryRef psDict;
    if (IORegistryEntryCreateCFProperties(batteryRegistry, &psDict, kCFAllocatorDefault, 0))
    {
        throw std::runtime_error("Something went wrong while reading the battery cycles.");
    }

    SInt32 cycles;
    CFNumberGetValue((CFNumberRef)CFDictionaryGetValue(psDict, CFSTR("CycleCount")), (CFNumberType)3, &cycles);

    return cycles;
}