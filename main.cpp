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

        int temp = system.getCpuTemp();

        std::cout << temp << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}