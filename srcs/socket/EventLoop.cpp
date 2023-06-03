#include <iostream>

#include "EventLoop.hpp"

EventLoop::EventLoop(Config &con) {
    Socket sock(con);
    this->_server = con._ServerBlockObject;
    this->_kqFd = kqueue();
    if (this->_kqFd == -1) {
        Config::iterator begin = con._ServerBlockObject.begin();
        Config::iterator end = con._ServerBlockObject.end();
		for ( ; begin != end ; begin++)
			close(begin->GetPort());
		exit(80);
    }
    this->_ChangeList = sock.GetChangeList();
    EventHandler();
}

void EventLoop::EventHandler() {
    int servcnt = this->_ChangeList.size(); // this->추가?
    struct kevent evntLst[16];
    struct kevent *curEvnts;
    int newEvnts;
    while (1) {
        newEvnts = kevent(_kqFd, &_ChangeList[0], _ChangeList.size(), evntLst,
                          16, NULL);
        _ChangeList.clear();
        for (int i = 0; i < newEvnts; i++) {
            curEvnts = &evntLst[i];
            if (curEvnts->filter == EVFILT_READ) {
                if (curEvnts->ident < (uintptr_t)servcnt + 3) { // 서버 소켓?
                    int clnt_fd;
                    if (curEvnts->udata != NULL) {
                        ServerBlock *sptr =
                            static_cast<ServerBlock *>(curEvnts->udata);
                        clnt_fd = accept(curEvnts->ident, 0, 0);
                        if (clnt_fd == -1)
                            continue;
                        struct linger linger_opt;
                        linger_opt.l_onoff = 1;
                        linger_opt.l_linger = 0; // TIME_WAIT 상태를 0초로 설정
                        if (setsockopt(clnt_fd, SOL_SOCKET, SO_LINGER,
                                       &linger_opt, sizeof(linger_opt)) < 0) {
                            std::cout << "setsockopt error" << std::endl;
                            close(clnt_fd);
                            continue;
                        }
                        int sigpipe_opt = 1;
                        if (setsockopt(clnt_fd, SOL_SOCKET, SO_NOSIGPIPE,
                                       &sigpipe_opt, sizeof(sigpipe_opt)) < 0) {
                            std::cout << "setsockopt error" << std::endl;
                            close(clnt_fd);
                            continue;
                        }
                        std::cout << sptr->GetPort() << "에 새 클라이언트("
                                  << clnt_fd << ") 연결" << std::endl;
                        fcntl(clnt_fd, F_SETFL, O_NONBLOCK);
                        struct kevent tmpEvnt;
                        EV_SET(&tmpEvnt, clnt_fd, EVFILT_READ,
                               EV_ADD | EV_ENABLE, 0, 0, 0);
                        _ChangeList.push_back(tmpEvnt);
                    }
                } else {
                    std::cout << "클라이언트(" << curEvnts->ident
                              << ")으로부터 request 들어옴" << std::endl;
                    HandleRequest(curEvnts);
                }
            } else if (curEvnts->filter == EVFILT_WRITE) {
                std::cout << "클라이언트(" << curEvnts->ident
                          << ")에게 Write 이벤트 발생" << std::endl;
                if (curEvnts->udata != NULL) { //ident = parentWrite[1];
                    std::cout << "?child write" << std::endl;
                    phpUdata *udata = static_cast<phpUdata *>(curEvnts->udata);
                    _response[udata->clnt_sock]->PhpChildWrite(curEvnts, _ChangeList);
                } else if (this->_response.find(curEvnts->ident) ==
                    this->_response.end()) {
                    // std::cout << "send failed non exist response" << std::endl;
                    close(curEvnts->ident);
                    EraseMemberMap(curEvnts->ident);
                } else
                    SendResponse(curEvnts);
            } else if (curEvnts->filter == EVFILT_PROC) {
                std::cout << Response::getCGI()[curEvnts->ident][0]
                          << " cgi 호출 끝" << std::endl;
                int sock_fd = Response::getCGI()[curEvnts->ident][0];
                _response[sock_fd]->PhpResultRead(curEvnts);
                struct kevent tmpEvnt;
                EV_SET(&tmpEvnt, sock_fd, EVFILT_WRITE,
                    EV_ADD | EV_ENABLE, 0, 0, curEvnts->udata);
                _ChangeList.push_back(tmpEvnt);


				Response::getCGI().erase(curEvnts->ident);
                // EraseMemberMap(Response::getCGI()[curEvnts->ident][0]);


                //if (_response[sock_fd]->generatePhpResponse(curEvnts, _ChangeList) == -1)
					// EraseMemberMap(Response::getCGI()[curEvnts->ident][0]);
					// Response::getCGI().erase(curEvnts->ident);
            } else {
                close(curEvnts->ident);
                EraseMemberMap(curEvnts->ident);
            }
        }
    }
}

void EventLoop::HandleRequest(struct kevent *curEvnts) {
    Request *reque = ParseRequest(curEvnts->ident, this->_request, _server);
    if (reque == NULL) {
        std::cout << curEvnts->ident << " : fin 요청" << std::endl;
        EraseMemberMap(curEvnts->ident);
        close(curEvnts->ident);
        return;
    }
    if ((reque->GetChunked() && reque->GetIsEnd()) || !reque->GetChunked())
        MakeResponse(curEvnts);
}

void EventLoop::MakeResponse(struct kevent *curEvnts) {
    int error;
    socklen_t error_len = sizeof(error);
    int ret =
        getsockopt(curEvnts->ident, SOL_SOCKET, SO_ERROR, &error, &error_len);
    if (ret == -1 || error != 0) {
        // Socket is dead, close it and return
        close(curEvnts->ident);
        EraseMemberMap(curEvnts->ident);
        //std::cout << "이미 뒤진 소켓 MakeResponse 에서 발견되다" << std::endl;
        return;
    }
    // std::cout << curEvnts->ident << " 에게 보낼 응답 msg 생성 " << std::endl;
    if (_request.find(curEvnts->ident) == _request.end()) {
        close(curEvnts->ident);
        EraseMemberMap(curEvnts->ident);
        return;
    }
    if (_response.find(curEvnts->ident) != _response.end())
        delete (_response[curEvnts->ident]);
    Request *reque = this->_request[curEvnts->ident];
    if (IsPhp(reque)) {
        // PhpStart(curEvnts, _ChangeList, this->_cli, _cgi);
        this->_response[curEvnts->ident] =
            new Response(reque, curEvnts, _ChangeList);
        if (this->_response[curEvnts->ident]->isDone()) {
            std::cout << "php failed" << std::endl;
            struct kevent tmpEvnt;
            EV_SET(&tmpEvnt, curEvnts->ident, EVFILT_WRITE, EV_ADD | EV_ENABLE,
                   0, 0, curEvnts->udata);
            _ChangeList.push_back(tmpEvnt);
        }
        // this->_response2[curEvnts->ident] = new Response(reque,
        // _ChangeList);
    } else {
        this->_response[curEvnts->ident] = new Response(reque);
        struct kevent tmpEvnt;
        EV_SET(&tmpEvnt, curEvnts->ident, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0,
               0, curEvnts->udata);
        _ChangeList.push_back(tmpEvnt);
    }
}

void EventLoop::SendResponse(struct kevent *curEvnts) {
    int error;
    socklen_t error_len = sizeof(error);
    int ret =
        getsockopt(curEvnts->ident, SOL_SOCKET, SO_ERROR, &error, &error_len);
    if (ret == -1 || error != 0) {
        // Socket is dead, close it and return
        close(curEvnts->ident);
        EraseMemberMap(curEvnts->ident);
        //std::cout << "이미 del 소켓 SendResponse 에서 발견되다" << std::endl;
        return;
    }
    std::cout << curEvnts->ident << " 에게 응답 msg 전송 " << std::endl;
    std::vector<char> resMsg;
    resMsg = _response[curEvnts->ident]->getResponseMessage();
    int res = send(curEvnts->ident, &resMsg[_offset[curEvnts->ident]],
                   resMsg.size() - _offset[curEvnts->ident], 0);
    if (res > 0) {
        _offset[curEvnts->ident] += res;
        return;
    }
    if (res == -1) {
        std::cout << "Write fail" << '\n';
        EraseMemberMap(curEvnts->ident);
        close(curEvnts->ident);
        return;
    } else {
        std::cout << "전송 완료" << std::endl;
    }
    std::cout << "Event Delete" << std::endl;
    struct kevent tmpEvnt;
    EV_SET(&tmpEvnt, curEvnts->ident, EVFILT_WRITE, EV_DELETE, 0, 0,
           curEvnts->udata);
    _ChangeList.push_back(tmpEvnt);
	if (!_request[curEvnts->ident]->getConnection())
		close(curEvnts->ident);
    EraseMemberMap(curEvnts->ident);
}

void EventLoop::EraseMemberMap(int key) {
    delete (this->_request[key]);
    delete (this->_response[key]);
    this->_request.erase(key);
    this->_response.erase(key);
    this->_offset.erase(key);
}

EventLoop::EventLoop() {}

EventLoop::~EventLoop() {}
