#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include <string>
#include <vector>
#include "ServerFiles.hpp"
#include <sys/types.h>
#include <sys/stat.h>

#define	CLIENT_SOCKET 0
#define	CHILD_OUT 1

class Request;

class LocationBlock;

class Response {
private:
	std::vector<char>  _responseMessage;
    std::vector<char> _statusHeaderMessage;
	std::vector<char> _bodyMessage;
	Request	*_request;
	static ServerFiles _serverFiles;
	std::string	_contentType;
	bool				_isDone;
	bool				_hasChildProc;
	static std::map<int, std::vector<int> > _cgi;
	typedef struct phpUdata
	{
		std::string *body;
		int			clnt_sock;
        int			pid;
        int 		child_write_zero;
    	int			parent_write_one; 
	} phpUdata;
	phpUdata _phpUdata;
	
	bool _isRedirection;
	std::string _redirectLocation;
	LocationBlock	&getLocationBlock();
public:
	Response(Request	*request);
	Response(Request *request, struct kevent *curEvnts, std::vector<struct kevent> &_ChangeList);
	Response(Response &response); //
	~Response();

	// getter
	std::vector<char> getResponseMessage();
    std::vector<char> getStatusHeaderMessage();
	std::vector<char> getBodyMessage();
	static std::map<int, std::vector<int> >	&getCGI();
	// std::vector<char> getResponseMessage();

	// setter
	void SetResponseMessage(std::string	responseMessage);
	void setResponseMessage(std::vector<char> &getResponseMessage);
    void setStatusHeaderMessage(std::vector<char> &getStatusHeaderMessage);
	void setBodyMessage(std::vector<char> &getBodyMessage);

	// checker
	bool	isAllowed(std::string	method);
	bool	isAutoIndex();
    bool 	isCGI();
	bool	isDirectory(const char *directory);
	bool	isRedirect();

	bool	isDone();
	bool	hasChildProc();

	void	getMethod();
	void	postMethod();
	void	deleteMethod();

	void	generateStatusLine();
	void	generateHeader();
	void	joinResponseMessage();
	void	generateAutoindex(const std::string& directory);
	void	generateErrorBody();
	void	generateDefaultErrorPage();

	std::string	fetchFilePath();

	// php 처리
	int	generatePhpResponse(struct kevent	*curEvnts);
	void	generatePhpHeader(std::string	phpResponse);
	void	generatePhpBody(std::string	&phpResponse);
	std::map<std::string, std::string> PhpEnvSet();
	bool	verifyFile(const char *filename);
	void	forkPhp(struct kevent *curEvents, std::vector<struct kevent> &_changeList);
	void	PhpResultRead(struct kevent *curEvnts);
	void	PhpChildWrite(struct kevent *curEvnts, std::vector<struct kevent> &_changeList);
};

#endif