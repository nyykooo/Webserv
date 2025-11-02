#include "../includes/Signals.hpp"

volatile sig_atomic_t g_shutdown = 0;

void signalHandler(int signum)
{
    (void)signum;
    g_shutdown = 1;
}

void setupSignals()
{
    signal(SIGQUIT, SIG_IGN);   // Ctrl+/
    signal(SIGINT, signalHandler);   // Ctrl+C
    signal(SIGTERM, signalHandler);  // kill/systemd
}