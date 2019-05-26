#ifndef SMCPPKIT_UTILS_H
#define SMCPPKIT_UTILS_H


#include <MacTypes.h>
#include <string>

class Utils {
public:
    /**
     * Takes a string and returns the given string as a FourCharCode.
     * @param givenString
     * @return The given string as a four char code.
     */
    static FourCharCode fourCharCodeFromString(const std::string& givenString) {
        char const *bytes = givenString.c_str();
        UInt32 byte0 = (unsigned)bytes[0] << (unsigned)24;
        UInt32 byte1 = (unsigned)bytes[1] << (unsigned)16;
        UInt32 byte2 = (unsigned)bytes[2] << (unsigned)8;
        UInt32 byte3 = (unsigned)bytes[3];

        return byte0 | byte1 | byte2 | byte3;
    }
};


#endif //SMCPPKIT_UTILS_H
