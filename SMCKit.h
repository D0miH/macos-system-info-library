#ifndef SMCPPKIT_SMCKIT_H
#define SMCPPKIT_SMCKIT_H

#include <IOKit/IOKitLib.h>
#include "SMCParamStruct.h"

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
     * @throws runtime_error    Throws a runtime error if closing the connection failed.
     */
    void close();

    /**
     * Makes a call to the SMC.
     */
    SMCParamStruct callSMC(SMCParamStruct givenStruct, SMCSelector);
};


#endif //SMCPPKIT_SMCKIT_H
