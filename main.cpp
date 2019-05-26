#include <iostream>
#include <thread>
#include "SMCKit.h"

int main() {
    SMCKit smc = SMCKit();
    smc.open();

    SMCKey key = SMCKey("TC0F", smc.types.SP78);
    SMCBytes readResult = { 0 };

    for(int i=0; i<10; i++) {
        smc.readKey(key, readResult);
        std::cout << (unsigned int)readResult[0]<< std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}