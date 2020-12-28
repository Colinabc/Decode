#include "ctrl_c.h"

bool STOP_ALL = false;

void handle_user_key(int key)
{
    std::cout << "[ CTRL C ] Stop all" << std::endl;
    STOP_ALL = true;
    //sleep(5); //3 seconds.
}

void install_interrupt_callback()
{
    struct sigaction sigIntHandler;
    sigIntHandler.sa_flags = 0;
    sigIntHandler.sa_handler = handle_user_key;
    sigemptyset(&sigIntHandler.sa_mask);
    sigaction(SIGINT, &sigIntHandler, NULL);
}
