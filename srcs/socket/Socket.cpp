#include <iostream>

#include "Socket.hpp"

Socket::Socket(Config& con)
{
	Config::iterator it ,its;
	it = con._ServerBlockObject.begin();
	its = con._ServerBlockObject.end();
	for ( ; it != its ; it++) {
		ServerInit(it, con);
	}
}

Socket::~Socket() {}

void Socket::ServerInit(Config::iterator it, Config &con)
{
	int servSock;
	struct sockaddr_in serv_addr;

	//이미 들어왔던 포트
	if (std::find(_portCheck.begin(), _portCheck.end(), it->GetPort()) != _portCheck.end()) {
		std::vector<std::string> serverNames = it->GetServerName();
		if (serverNames.empty()) {
			std::cout << "serverName setting err" << std::endl;
			Config::iterator begin = con._ServerBlockObject.begin();
			for ( ; it != begin ; it--)
				close(it->GetPort());
			exit(80);
		}
		//서버네임이 중복인지 체크
		else {
			std::vector<std::string>::iterator b, e;
			b = serverNames.begin();
			e = serverNames.end();
			for ( ; b != e ; b++) {
				if (std::find(_dupPort[it->GetPort()].begin(), _dupPort[it->GetPort()].end(), *b) != _dupPort[it->GetPort()].end()) {
					std::cout << "duplicate port err" << std::endl;
					Config::iterator begin = con._ServerBlockObject.begin();
					for ( ; it != begin ; it--)
						close(it->GetPort());
					exit(80);
				}
			}
		}
		//중복이 아님
		if (serverNames.empty())
			_dupPort[it->GetPort()].push_back("_");
		else {
			for (size_t i = 0 ; i < serverNames.size() ; i++) {
				_dupPort[it->GetPort()].push_back(serverNames[i]);
			}
		}
		return ;
	}
	//들어온적 없는 포트
	else {
		std::vector<std::string> serverNames = it->GetServerName();
		if (serverNames.empty())
			_dupPort[it->GetPort()].push_back("_");
		else {
			for (size_t i = 0 ; i < serverNames.size() ; i++) {
				_dupPort[it->GetPort()].push_back(serverNames[i]);
			}
		}
	}

	_portCheck.push_back(it->GetPort());
	if ((servSock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		std::cout << "socket err" << std::endl;
		Config::iterator begin = con._ServerBlockObject.begin();
		for ( ; it != begin ; it--) {
			close(it->GetPort());
		}
		close(it->GetPort());
		exit(80);
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(it->GetPort());

	//서버 종료후 바로 재시작시 bind()함수에서 중복 포트 에러 처리를 위한 소켓 세팅 로직
	// int option = 1;
    // if (setsockopt(servSock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) < 0) {
    //     std::cout << "setsockopt error" << std::endl;
	// 	exit(1);
    // }
	// TIME_WAIT 상태인 소켓 재사용 가능 (테스트가 끝난면 삭제 ㄱㄱ)

	if (bind(servSock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		if (errno == EADDRINUSE) {
			std::cout << "duplicate port err" << std::endl;
			Config::iterator begin = con._ServerBlockObject.begin();
			for ( ; it != begin ; it--) {
				close(it->GetPort());
			}
			close(it->GetPort());
			exit(80);
		}
		else
		{
			std::cout << "bind err" << std::endl;
			exit(1);
		}
	}
	if (listen(servSock, 1000) < 0)
	{
		std::cout << "listen err" << std::endl;
		exit(1);
	}
	AddSocketEvent(servSock, *it);
}


//changelist 에 서버 소켓에 대한 이벤트 정보 저장
void Socket::AddSocketEvent(int servSock, ServerBlock& ServerBlock)
{
	struct kevent tmpEvnt;
	void *ptr = static_cast<void *>(&ServerBlock);
	EV_SET(&tmpEvnt, servSock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, ptr);
	this->_changList.push_back(tmpEvnt);
}

std::vector<struct kevent>& Socket::GetChangeList()
{
	return (this->_changList);
}
