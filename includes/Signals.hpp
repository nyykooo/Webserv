#ifndef SIGNALS_HPP
#define SIGNALS_HPP

#include <csignal>      // signal, sig_atomic_t

extern volatile sig_atomic_t g_shutdown;

void signalHandler(int signum);
void setupSignals();

#endif