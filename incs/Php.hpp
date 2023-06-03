#ifndef PHP_HPP
#define PHP_HPP

#include "Config.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Socket.hpp"
#include <fcntl.h>
#include <map>
#include <unistd.h>

void PhpStart(struct kevent *curEvnts, std::vector<struct kevent> &_ChangeList,
              std::map<int, Request *> &_cli,
              std::map<int, std::vector<int> > &_cgi);
bool IsPhp(Request *reque);
std::vector<char> PhpResult(struct kevent *curEvnts,
                            std::vector<struct kevent> &_ChangeList,
                            std::map<int, Request *> &_cli,
                            std::map<int, std::vector<int> > &_cgi);
std::map<std::string, std::string> PhpEnvSet(struct kevent *curEvnts,
               std::map<int, Request *> &_cli,
               std::map<int, std::vector<int> > &_cgi);
#endif
