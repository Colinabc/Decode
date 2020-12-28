#ifndef CTRL_C_H
#define CTRL_C_H
// c signal
#include <unistd.h>
#include <signal.h>
// c++
#include <iostream>

extern bool STOP_ALL;

void handle_user_key(int key);

void install_interrupt_callback();

#endif
