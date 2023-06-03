#include "Request.hpp"
#include "get_next_line.h"
#include "utils.hpp"
#include "StateCode.hpp"
#include "ServerBlock.hpp"
#include <algorithm>
#include <map>

ServerBlock	*FindServer(std::vector<ServerBlock> &servers, Request	*request) {
	int	idx = -1;
    std::vector<std::string>::iterator iter;


    // request 포인터가 NULL인 경우 예외 처리
    if (request == NULL) {
        return NULL;
    }
	// port가 같은 객체가 있는지 확인
    for (size_t i = 0; i < servers.size(); i++) {
        if (servers[i].GetPort() == request->GetHostPort()) {
            if (idx == -1)
                idx = static_cast<int>(i);
            // std::find 대신 for 루프와 문자열 비교를 통해 조건 검사
            for (size_t j = 0; j < servers[i].GetServerName().size(); j++) {
                if (servers[i].GetServerName()[j] == request->GetHostName()) {
					return &servers[i];
                }
            }
        }
    }
    if (idx == -1) {
        return NULL;
    }
    return &servers[idx];
}

Request *ParseRequest(int fd, std::map<int, Request *> &clients,
					  std::vector<ServerBlock> &servers) {
	std::stringstream ss;

	std::map<int, Request *>::iterator clientsIterator = clients.find(fd);
	if (clientsIterator == clients.end()) {
		try {
			char *line = get_next_line(fd);
			if (line == NULL)
				return NULL;
			while (line) {
				ss << line;
				std::string tmp(line);
				free(line);
				if (tmp == CRLF)
					break;
				line = get_next_line(fd);
			}
			std::cout << ss.str() << std::endl;
		}
		catch (ReadFail &e) {
			return NULL;
		}
		Request *request = new Request(fd, ss);
		if (request->GetErrorCode() == 600) {
			delete(request);
			return NULL;
		}
		request->SetServer(FindServer(servers, request));
		try {
			if (FindServer(servers, request) == NULL) {
				delete request;
				return NULL;
			}
			clients.insert(std::pair<int, Request *>(fd, request));
			if (request->getBinary().size() > request->GetServer().GetClientMaxBodySize())
				throw BodySizeError();
		} catch (const StateCode &e) {
			request->SetErrorCode(e._errorCode);
			request->SetErrorMessages(e.what());
		}
    } else {
        Request *request = clients[fd];
		try {
			if (request->GetMethod() == "POST")
				request->readBody(fd);
		} catch (ReadFail &e){
			return NULL;
		} catch (const StateCode &e) {
			request->SetErrorCode(e._errorCode);
			request->SetErrorMessages(e.what());
		}
	}
	return clients[fd];
}