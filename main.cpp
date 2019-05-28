#include <iostream>
#include <thread>
#include "SMCKit.h"

int main() {
    SMCKit smc = SMCKit();
    smc.open();

    for(int i=0; i<10; i++) {
        bool temp = smc.isOnAC();
        std::cout << temp << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}