#ifndef SOCKET_HPP
# define SOCKET_HPP

//sockaddr_in 
#include <arpa/inet.h>
//evset
#include <sys/event.h>
//socket and kqueue
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include"Config.hpp"

class Socket
{
private:
	std::vector<int> _portCheck;
	std::map<int, std::vector<std::string> > _dupPort;
	std::vector<struct kevent> _changList;
	/* data */
	void ServerInit(Config::iterator it, Config& con);
	void AddSocketEvent(int servSock, ServerBlock& ServerBlock);

public:
	Socket(Config& con);
	~Socket();
	std::vector<struct kevent>& GetChangeList();
};

#endif
