#include <iostream>
#include <thread>
#include <numeric>
#include "SMCKit.h"

int main()
{
    SMCKit smc = SMCKit();
    smc.open();

    for (int i = 0; i < 10; i++)
    {

        std::vector<float> cpuUsage = smc.getCpuUsage();

        std::cout << "User: " << cpuUsage[0] << " System: " << cpuUsage[1] << " Idle: " << cpuUsage[2] << " Nice: " << cpuUsage[3] << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}