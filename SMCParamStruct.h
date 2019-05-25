#ifndef SMCPPKIT_SMCPARAMSTRUCT_H
#define SMCPPKIT_SMCPARAMSTRUCT_H

#include <IOKit/IOKitLib.h>

/**
 * These enum and structs are defined in the Apple PowerManagement project:
 * https://opensource.apple.com/source/PowerManagement/PowerManagement-211/pmconfigd/PrivateLib.c.auto.html
 */

enum SMCResult : UInt8 {
    kSMCSuccess = 0,
    kSMCError = 1,
    kSMCKeyNotFound = 132
};

enum SMCSelector : UInt8 {
    kSMCHandleYPCEvent = 2,
    kSMCReadKey = 5,
    kSMCWriteKey = 6,
    kSMCGetKeyFromIndex = 8,
    kSMCGetKeyInfo = 9
};

typedef UInt8 SMCBytes[32];

typedef struct {
    unsigned char major = 0;
    unsigned char minor = 0;
    unsigned char build = 0;
    unsigned char reserved = 0;
    unsigned short release = 0;
} smc_version_t;

typedef struct {
    UInt16 version = 0;
    UInt16 length = 0;
    UInt16 cpuPLimit = 0;
    UInt16 gpuPLimit = 0;
    UInt16 memPLimit = 0;
} smc_limit_data_t;

typedef struct {
    IOByteCount dataSize = 0;
    UInt32 dataType = 0;
    UInt8 dataAttributes = 0;
} smc_key_info_data_t;

struct SMCParamStruct {

    UInt32 key = 0;

    smc_version_t vers = smc_version_t();

    smc_limit_data_t pLimitData = smc_limit_data_t();

    smc_key_info_data_t keyInfo = smc_key_info_data_t();

    UInt8 result = 0;

    UInt8 status = 0;

    UInt8 data8 = 0;

    UInt32 data32 = 0;

    SMCBytes bytes = { 0 };
};

#endif //SMCPPKIT_SMCPARAMSTRUCT_H
