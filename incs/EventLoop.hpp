#ifndef EVENTLOOP_HPP
#define EVENTLOOP_HPP

#include "Config.hpp"
#include "Php.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Socket.hpp"
#include <array>
#include <fcntl.h>
#include <map>
#include <unistd.h>

class EventLoop {
  private:
    int _kqFd;
    std::vector<struct kevent> _ChangeList;

    std::map<int, Request *> _request;
    std::map<int, Response *> _response;
    std::map<int, int> _offset;
    std::vector<ServerBlock> _server;
    typedef struct phpUdata {
        std::string *body;
        int clnt_sock;
        int pid;
        int child_write_zero;
        int parent_write_one;
    } phpUdata;
    EventLoop();

  public:
    EventLoop(Config &con);
    ~EventLoop();
    void MakeHtmlFile(Config &con);
    void EventHandler();
    void HandleRequest(struct kevent *curEvnts);
    void MakeResponse(struct kevent *curEvnts);
    void SendResponse(struct kevent *curEvnts);
    void EraseMemberMap(int key);
};

#endif
