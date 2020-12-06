#include <beauty/beauty.hpp>

#include <iostream>

int main()
{
#ifndef WIN32
    // Catch the small signals
    beauty::signal({SIGUSR1, SIGUSR2}, [](int s) {
        std::cout << "Shot miss..." << std::endl;
    });

    // Catch the big one
    beauty::signal(SIGINT, [](int s) {
        std::cout << "Head shot !" << std::endl;
        beauty::stop();
    });

    // Wait for the end
    beauty::wait();
#endif
}
