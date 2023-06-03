#include "Config.hpp"
#include "EventLoop.hpp"
#include "Socket.hpp"
#include <iostream>
#include <string>

// 포트야 안아프게 닫혀보자
#include <signal.h>

void sigint_handler(int signum) {
    std::cout << "\nReceived SIGINT signal, closing all socket and exiting..."
              << std::endl;
    int i = 3;
    while (i != 2000) {
        close(i);
        i++;
    }
    exit(signum);
}
// void leaks() { system("leaks webserv"); }

int main(int ac, char **av) {
    // at exit(leaks);₩

    signal(SIGKILL, sigint_handler);
    if (ac != 2) {
        std::cout << "Usage ./webserv [Configuration]" << std::endl;
        exit(1);
    }
    Config con(static_cast<std::string>(av[1]));
    con.LocationSort();
    EventLoop loop(con);
}
