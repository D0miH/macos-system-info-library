//
// Created by Dominik on 2019-05-24.
//

#ifndef SMCKITCPP_SMCKIT_H
#define SMCKITCPP_SMCKIT_H

#include <IOKit/IOKitLib.h>

/**
 * These enum and structs are defined in the Apple PowerManagement project:
 * https://opensource.apple.com/source/PowerManagement/PowerManagement-211/pmconfigd/PrivateLib.c.auto.html
 */
enum SMCResult: UInt8 {
    kSMCSuccess = 0,
    kSMCError = 1,
    kSMCKeyNotFound = 132
};

enum SMCSelector: UInt8 {
    kSMCHandleYPCEvent = 2,
    kSMCReadKey = 5,
    kSMCWriteKey = 6,
    kSMCGetKeyFromIndex = 8,
    kSMCGetKeyInfo = 9
};

typedef struct SMCVersion {
    unsigned char major;
    unsigned char minor;
    unsigned char build;
    unsigned char reserved;
    unsigned short release;
} SMCVersion;

typedef struct SMCLimitData {
    UInt16 version;
    UInt16 length;
    UInt16 cpuPLimit;
    UInt16 gpuPLimit;
    UInt16 memPLimit;
} SMCLimitData;

typedef struct SMCKeyInfoData {
    IOByteCount dataSize;
    UInt32 dataType;
    UInt8 dataAttributes;
} SMCKeyInfoData;

class SMCKit {
private:
    io_connect_t  connectionHandle;

public:
    ~SMCKit() {
        close();
    }

    /**
     * Opens a connection to the SMC driver.
     * @throws runtime_error    Throws a runtime error if the connection can not be made.
     */
    void open();

    /**
     * Closes the connection to the SMC driver.
     */
    void close();

    /**
     * Makes a call to the SMC.
     */
    void callSMC();
};


#endif //SMCKITCPP_SMCKIT_H
