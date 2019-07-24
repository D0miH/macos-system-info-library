#ifndef SMCPPKIT_UTILS_H
#define SMCPPKIT_UTILS_H

#include <MacTypes.h>
#include <cstring>

class Utils
{
public:
    /**
     * Takes a string and returns the given string as a FourCharCode.
     * @param givenString   The given string
     * @return The four char code
     */
    static FourCharCode stringToFourCharCode(const std::string &givenString)
    {
        const char *bytes = givenString.c_str();
        UInt32 byte0 = (unsigned)bytes[0] << (unsigned)24;
        UInt32 byte1 = (unsigned)bytes[1] << (unsigned)16;
        UInt32 byte2 = (unsigned)bytes[2] << (unsigned)8;
        UInt32 byte3 = (unsigned)bytes[3];

        return byte0 | byte1 | byte2 | byte3;
    }

    /**
     * Takes a FourCharCode and returns the given code as a string.
     * @param givenCode The given FourCharCode
     * @return  The four char code
     */
    static std::string fourCharCodeToString(FourCharCode givenCode)
    {
        char byte0 = givenCode >> (unsigned)24;
        char byte1 = givenCode >> (unsigned)16;
        char byte2 = givenCode >> (unsigned)8;
        char byte3 = givenCode;

        return std::string({byte0, byte1, byte2, byte3});
    }

    /**
     * Converts a given value from the fpe2 data type to int.
     * @param value     The given value as fpe2 type.
     * @return  The corresponding integer value.
     */
    static int fpe2ToInt(UInt8 value[2])
    {
        return (value[0] << (unsigned)6) + (value[1] >> (unsigned)2);
    }

    /**
     * Converts a given value from the flt data type to int.
     * @param value     The given value as flt type.
     * @return  The corresponding integer value.
     */
    static int fltToInt(UInt8 value[4])
    {
        float resultValue = 0;
        std::memcpy(&resultValue, value, 4);
        return (int)resultValue;
    }
};

#endif //SMCPPKIT_UTILS_H
