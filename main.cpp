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

        std::vector<float> memUsage = system.getMemoryUsage();

        std::cout << "free: " << memUsage[0] << " active: " << memUsage[1] << " inactive: " << memUsage[2] << " wired: " << memUsage[3] << " compressed: " << memUsage[4] << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}