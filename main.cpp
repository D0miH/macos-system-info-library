#include <iostream>
#include <thread>
#include <numeric>
#include "System.h"

int main()
{
    System system = System();
    system.open();

    for (int i = 0; i < 10; i++)
    {

        float mem = system.getTotalMemory();

        std::cout << mem << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}