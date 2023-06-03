#include "Response.hpp"
#include "Request.hpp"
#include "ServerBlock.hpp"
#include "StateCode.hpp"
#include "dirent.h"
#include "utils.hpp"
#include <cstdio>
#include <iostream>
#include <signal.h>
#include <sys/event.h>
#include <unistd.h>

Response::Response(Request *request)
    : _request(request), _isDone(false), _hasChildProc(false) {
    try {
        if (_request->GetContentType() == "multipart/form-data") {
            std::string path = getLocationBlock().GetRoot() +
                               getLocationBlock().GetUploadPass();
            for (size_t i = 0; i < _request->getFormData().size(); i++) {
                std::string filename;
                if (_request->getFormData()[i]->getFileName().size())
                    filename =
                        path + "/" + _request->getFormData()[i]->getFileName();
                else
                    filename = path + "/" +
                               _request->getFormData()[i]->GetContentType();
                _serverFiles.saveFile(
                    filename, _request->getFormData()[i]->getBinary(),
                    _request->getFormData()[i]->GetContentType());
            }
            _request->SetErrorCode(Created);
        }
        if (request->GetErrorCode() / 100 != 2)
            generateErrorBody();
        else if (request->GetMethod() == "GET")
            getMethod();
        else if (request->GetMethod() == "POST")
            postMethod();
        else if (request->GetMethod() == "DELETE")
            deleteMethod();
    } catch (NotExist &e) {
        _request->SetErrorCode(e._errorCode);
        _request->SetErrorMessages(e.what());
        try {
			// target 유효성 검증 조건문 추가
            if (isAutoIndex() && verifyFile(_request->GetTarget().c_str()))
            {
                generateAutoindex(
                    _request->GetServer().GetRoot()); // autoindex 처리
            }
            else {

                generateErrorBody();
            }
        } catch (NotExist &e) {
            generateErrorBody();
        }
        _contentType = "text/html";
        _isDone = true;
    } catch (StateCode &e) {
        _request->SetErrorCode(e._errorCode);
        _request->SetErrorMessages(e.what());
        generateErrorBody();
        _contentType = "text/html";
    }
    generateStatusLine();
    generateHeader();
    joinResponseMessage();
    _isDone = true;
}

Response::Response(Request *request, struct kevent *curEvnts,
                   std::vector<struct kevent> &_ChangeList)
    : _request(request), _isDone(false), _hasChildProc(false) {
    try {
        if (request->GetErrorCode() / 100 != 2) {
            generateErrorBody();
            generateStatusLine();
            generateHeader();
            joinResponseMessage();
            _isDone = true;
        }
        std::string filename = fetchFilePath();
        verifyFile(filename.c_str());
        forkPhp(curEvnts, _ChangeList);
    } catch (NotExist &e) {
        _request->SetErrorCode(e._errorCode);
        _request->SetErrorMessages(e.what());
        try {

            if (isAutoIndex() && verifyFile(_request->GetTarget().c_str()))
            {
                generateAutoindex(
                    _request->GetServer().GetRoot()); // autoindex 처리
                }
            else {
                generateErrorBody();
            }
        } catch (NotExist &e) {
            generateErrorBody();
        }
        _contentType = "text/html";
        _isDone = true;
        generateStatusLine();
        generateHeader();
        joinResponseMessage();
    } catch (StateCode &e) {
        _request->SetErrorCode(e._errorCode);
        _request->SetErrorMessages(e.what());
        generateErrorBody();
        _contentType = "text/html";
        generateStatusLine();
        generateHeader();
        joinResponseMessage();
        _isDone = true;
    }

    // 127.0.0.1/aaaaa.php
}

Response::~Response() {}

Response::Response(Response &response) {
    this->_responseMessage = response.getResponseMessage();
}

ServerFiles Response::_serverFiles = ServerFiles();

std::map<int, std::vector<int> > Response::_cgi;

bool Response::isDone() { return _isDone; }

std::string Response::fetchFilePath() {
    std::string location = getLocationBlock().GetLocationTarget();
    std::string target = _request->GetTarget().substr(location.size());
    if (target.size() && !(*(target.end() - 1) == '/' && target.size() == 1)) {
        if (target[0] != '/') {
            target.insert(0, "/");
        }
        target.insert(0, getLocationBlock().GetRoot());
    } else {
		if (_request->GetMethod() == "DELETE")
			throw NotExist();
        struct stat buffer;
        std::vector<std::string> index = getLocationBlock().GetIndex();

        for (size_t i = 0; i < index.size(); i++) {
            target = getLocationBlock().GetRoot();
            if (index[i][0] != '/') {
                target += "/";
            }
            target += index[i];
            if (stat(target.c_str(), &buffer) != 0)
                continue;
            else {
                return target;
            }
        }
        // redirect인지 아닌지에 따라서 throw
        if (isRedirect() == false && isAutoIndex() == false) {
            throw NotExist();
        }
    }

    return target;
}

int Response::generatePhpResponse(struct kevent *curEvnts) {
    // map
    waitpid(curEvnts->ident, 0, WNOHANG);
    _hasChildProc = false;
    std::stringstream ss;
    char buf[1024];
    int read_size = 1;
    close(curEvnts->ident);

    while (true) {
        read_size = read(_cgi[curEvnts->ident][CHILD_OUT], buf, 1024);
        if (read_size == 0) {
            break;
        } else if (read_size == -1) {
            close(curEvnts->ident);
            close(_cgi[curEvnts->ident][CHILD_OUT]);
            throw ServerError("php read failed");
        }
        buf[read_size] = 0;
        ss << buf;
    }
    close(_cgi[curEvnts->ident][CHILD_OUT]); //
    std::string phpResponse = ss.str();
    generateStatusLine();
    generatePhpBody(phpResponse);
    generatePhpHeader(phpResponse);
    joinResponseMessage();
    _isDone = true;
    return 0;
}

void Response::generatePhpBody(std::string &phpResponse) {
    size_t len = phpResponse.find("\r\n\r\n");
    if (len == std::string::npos) {
        return;
    }
    std::string tmp = phpResponse.substr(len + 4);
    phpResponse.erase(phpResponse.begin() + len, phpResponse.end());
    tmp += "\r\n\r\n";
    std::copy(tmp.begin(), tmp.end(), std::back_inserter(_bodyMessage));
}

void Response::generatePhpHeader(std::string phpResponse) {
    std::stringstream ss;

    ss << phpResponse << CRLF;
    ss << "Server: Webserv" << CRLF;
    if (_bodyMessage.size()) {
        ss << "Content-Length: " << _bodyMessage.size() << CRLF;
    }
    ss << CRLF;
    std::string line = ss.str();
    std::copy(line.begin(), line.end(),
              std::back_inserter(_statusHeaderMessage));
}

LocationBlock &Response::getLocationBlock() {
    ServerBlock Block;
    int longestIndex = -1;
	size_t longestLength = 0;

    std::string target = _request->GetTarget();
    for (size_t i = 0; i < _request->GetServer().GetLocation().size(); i++) {
        std::string locationTarget =
            _request->GetServer().GetLocation()[i].GetLocationTarget();
        if (!strncmp(locationTarget.c_str(), target.c_str(),
                     locationTarget.size())) {
            if (locationTarget.size() > longestLength) {
				if (locationTarget != "/" && target.size() > locationTarget.size() && target[locationTarget.size()] != '/')
					continue ;
				longestLength = locationTarget.size();
                longestIndex = static_cast<int>(i);
			}
        }
    }
    if (longestIndex != -1) {
        return _request->GetServer().GetLocation()[longestIndex];
	}
    throw NotExist();
}

bool Response::isCGI() {
    LocationBlock block = getLocationBlock();
    if (block.GetLocationTarget().find(".php"))
        return true;
    return false;
}

bool Response::isAutoIndex() {
    LocationBlock block = getLocationBlock();
    if (block.GetAutoIndex() == 1)
        return true;
    return false;
}

bool Response::isAllowed(std::string method) {
    std::vector<std::string> limit_except = getLocationBlock().GetLimitExcept();

    if (limit_except.size() == 0)
        return true;
    else if (find(limit_except.begin(), limit_except.end(), method) ==
             limit_except.end())
        return false;
    else
        return true;
}

bool Response::isRedirect() {

    _isRedirection = false;

    // tmp는 return 의 경로 ex) return 302 /return_dir2;
    std::string tmp = getLocationBlock().GetReturn().second;
    tmp.insert(0, "http://");
    // prefix로 헤더 필드 Location: htt:///return_dir2로 url 만들어줌
    std::string prefix = "Location: ";
    std::vector<char> redirectLocation(prefix.begin(), prefix.end());

    redirectLocation.insert(redirectLocation.end(), tmp.begin(), tmp.end());
    std::string str(redirectLocation.begin(), redirectLocation.end());
    _redirectLocation = str;

    if (!getLocationBlock().GetLocationTarget().empty())
    {
        if (getLocationBlock().GetReturn().first == 302 && getLocationBlock().GetLocationTarget() == _request->GetTarget()) {
            // std::cout << "성공"  << std::endl;
            _request->SetErrorCode(302);
            _request->SetErrorMessages("Moved Temporarily");
            _isRedirection = true;
            // std::cout << "여기서 404 ?22" << std::endl;
        };
    }
    else
    {
			// std::cout << "여기서 404 ?" << std::endl;
            _request->SetErrorCode(404);
            _isRedirection = false;
    }
    return _isRedirection;
}





void Response::generateAutoindex(const std::string &directory) {
    std::string dir_path = directory;

    DIR *dir = opendir(dir_path.c_str());
    struct dirent *entry;
    std::stringstream ss;
    std::string tmp;
    ss << "<!DOCTYPE html>\n<html>\n<head>\n<title>Index of " << dir_path
       << "</title>\n</head>\n<body>\n<h1>Index of " << dir_path
       << "</h1>\n<hr>\n<table>\n<tr><th>Name</th></tr>\n";

    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name; // 파일/디렉토리 이름
        if (_request->GetErrorCode() != NotFound) {
            if (name == "..") {
                name.clear();
                size_t found = _request->GetTarget().find_last_of("/");
                name = _request->GetTarget().substr(0, found);
            } else {
                if (name == ".")
                    name.clear();
                else if (name[0] != '/')
                    name.insert(0, "/");
                name.insert(0, _request->GetTarget());
            }
        }
        ss << "<tr><td><a href=\"" << name << "\">";
        ss << std::string(entry->d_name) << "</a></td></tr>\n";
    }

    ss << "</table>\n<hr>\n</body>\n</html>\n";
    tmp = ss.str();

    closedir(dir);
    _bodyMessage.clear();
    std::copy(tmp.begin(), tmp.end(), std::back_inserter(_bodyMessage));
}

void Response::generateStatusLine() {
    std::stringstream ss;

    ss << "HTTP/1.1 " << _request->GetErrorCode() << ' '
       << _request->GetErrorMessages().substr(
              0, _request->GetErrorMessages().find(" : "))
       << CRLF;
    std::string line = ss.str();
    std::copy(line.begin(), line.end(),
              std::back_inserter(_statusHeaderMessage));
}

void Response::generateHeader() {
    std::stringstream ss;

    ss << "Server: Webserv" << CRLF;
    if (_bodyMessage.size()) {
        ss << "Content-Type: " << _contentType << CRLF;
        ss << "Content-Length: " << _bodyMessage.size() << CRLF;
    }
    if (_request->GetErrorCode() == NotAllowed) {
        std::vector<std::string> limit_except =
            getLocationBlock().GetLimitExcept();
        ss << "Allow: ";
        for (size_t i = 0; i < limit_except.size(); ++i) {
            if (i)
                ss << ", ";
            ss << limit_except[i];
        }
        ss << CRLF;
    }
    if (_request->GetErrorCode() / 100 == 3)
        ss << _redirectLocation << CRLF;

    ss << CRLF;
    std::string line = ss.str();
    std::copy(line.begin(), line.end(),
              std::back_inserter(_statusHeaderMessage));

    std::string str(_statusHeaderMessage.begin(), _statusHeaderMessage.end());
}

void Response::generateDefaultErrorPage() {
    std::vector<char> vec = _serverFiles.getFile("./html/errorpage.html");
    std::string str(vec.begin(), vec.end());
    str.push_back('\0');
    std::stringstream ss;
    ss << _request->GetErrorCode();
    size_t header = str.find("errorCode");

    if (header != std::string::npos) {
        str.replace(header, std::string("errorCode").length(), ss.str());
    }
    size_t body = str.find("errorMessageShort");
    if (body != std::string::npos) {
        str.replace(body, std::string("errorMessageShort").length(),
                    _request->GetErrorMessages().substr(
                        0, _request->GetErrorMessages().find(" : ")));
    }
    size_t pos = str.find("errorMessage");
    if (pos != std::string::npos) {
        str.replace(pos, std::string("errorMessage").length(),
                    _request->GetErrorMessages().substr(
                        _request->GetErrorMessages().find(" : ") + 3));
    }
    _bodyMessage.clear();
    std::copy(str.begin(), str.end(), std::back_inserter(_bodyMessage));
}

void Response::generateErrorBody() {
    std::map<int, std::string>::iterator it =
        _request->GetServer().GetErrorPage().find(_request->GetErrorCode());
    try {
        if (it != _request->GetServer().GetErrorPage().end()) {
            std::string filename = _request->GetServer().GetRoot() + it->second;
            _bodyMessage = _serverFiles.getFile(filename);
        } else
            generateDefaultErrorPage();
    } catch (const StateCode &e) {
        generateDefaultErrorPage();
    }
    _contentType = "text/html";
}

void Response::joinResponseMessage() {
    std::copy(_statusHeaderMessage.begin(), _statusHeaderMessage.end(),
              std::back_inserter(_responseMessage));
    std::copy(_bodyMessage.begin(), _bodyMessage.end(),
              std::back_inserter(_responseMessage));
    std::string crlf(CRLF);
    std::copy(crlf.begin(), crlf.end(), std::back_inserter(_responseMessage));
    std::copy(crlf.begin(), crlf.end(), std::back_inserter(_responseMessage));
}

bool Response::isDirectory(const char *directory) {
    struct stat buffer;

    if (stat(directory, &buffer) != 0) {
        return false;
    }
    return S_ISDIR(buffer.st_mode);
}

bool Response::verifyFile(const char *filename) {
    struct stat buffer;

    if( isRedirect() == true)
	{
		return true;
	}
        // get 요청일 때 쿼리 스트링 짜르는 부분///////////
        std::string _filename(filename);
    size_t idx;
    idx = _filename.find(".php?");
    if (idx != std::string::npos) {
        _filename.erase(idx + 4, _filename.length());
        filename = _filename.c_str();
    }

    if (stat(filename, &buffer) != 0) {
        if (errno == EACCES)
            throw PermissionDenied();
        // ~/hello42.html 이후에 /hello42.html/42라는 잘못된 요청에 대해서 유효성 검증
        else if ((errno == ENOENT || errno == 20) && isRedirect() == false) {
            throw NotExist();
        }
        throw ServerError(strerror(errno));
    }
    return true;
}

void Response::getMethod() {
    std::string fileToRead;
    if (!isAllowed("GET"))
        throw MethodNotAllowed();
    // 파일 경로 가져오기
    fileToRead = fetchFilePath();
    // 디렉토리인지 체크
	verifyFile(fileToRead.c_str());
    if (isDirectory(fileToRead.c_str())) {
        if (!isAutoIndex())
            throw PermissionDenied();
        generateAutoindex(fileToRead);
        return;
    }

    // 파일 가져오는 부분

    if (isRedirect())
        _bodyMessage.clear();
    else
        _bodyMessage = _serverFiles.getFile(fileToRead);
    // Content-Type 설정하는 부분
    // 확장자가 .로 끝날경우 text/plain
    if (fileToRead.find(".") == std::string::npos ||
        fileToRead.find(".") == fileToRead.size() - 1)
        _contentType = "text/plain";
    else {
        std::string extention =
            fileToRead.substr((fileToRead.rfind(".") + 1 <= fileToRead.size()
                                   ? fileToRead.rfind(".") + 1
                                   : -1));
        if (!extention.compare("png")) // png 확장자 확인
            _contentType = "image/png";
        else if (!extention.compare("txt"))
            _contentType = "text/plain";
        else
            _contentType =
                "text/" +
                extention; // 그 외라면 오는 확장자 맞춰서 콘텐츠 타입 설정
    }
}

void Response::postMethod() {
    std::string fileToRead;
    if (!isAllowed("POST"))
        throw MethodNotAllowed();
    fileToRead = fetchFilePath();
	verifyFile(fileToRead.c_str());
    if (isDirectory(fileToRead.c_str())) {
        if (!isAutoIndex())
            throw PermissionDenied();
        generateAutoindex(fileToRead);
        return;
    }
    _bodyMessage = _serverFiles.getFile(fileToRead);
    // 확장자가 .로 끝날경우 text/plain
    if (fileToRead.find(".") == std::string::npos ||
        fileToRead.find(".") == fileToRead.size() - 1)
        _contentType = "text/plain";
    else {
        std::string extention =
            fileToRead.substr((fileToRead.rfind(".") + 1 <= fileToRead.size()
                                   ? fileToRead.rfind(".") + 1
                                   : -1));
        if (!extention.compare(".png")) // png 확장자 확인
            _contentType = "image/png";
        else if (!extention.compare("txt"))
            _contentType = "text/plain";
        else
            _contentType =
                "text/" +
                extention; // 그 외라면 오는 확장자 맞춰서 콘텐츠 타입 설정
    }
}

void Response::deleteMethod() {
    std::string fileToRead;
    if (!isAllowed("DELETE"))
        throw MethodNotAllowed();
	//if (_request->GetTarget() == getLocationBlock().GetLocationTarget())
    //    throw NotExist();
    fileToRead = fetchFilePath();
	verifyFile(fileToRead.c_str());
    if (remove(fileToRead.c_str()) != 0) {
        throw NotExist();
    }
	_request->SetErrorCode(NoContent);
}

std::map<std::string, std::string> Response::PhpEnvSet() {
    std::map<std::string, std::string> _envMap;
    std::string _method = _request->GetMethod();
    std::string content_length = "CONTENT_LENGTH=";
    // std::string content_type = "CONTENT_TYPE=";
    std::string script_filename = "SCRIPT_FILENAME=";
    std::string redirect_status = "REDIRECT_STATUS=";
    std::string script_name = "SCRIPT_NAME=";
    std::string request_uri = "REQUEST_URI=";
    std::string document_root = "DOCUMENT_ROOT=";
    std::string request_method = "REQUEST_METHOD=";
    std::string query_string = "QUERY_STRING="; //

    std::string _target =
        _request->GetTarget(); // filename임 post요청의 target이니
                               // index.php가 되겠다.
    std::ostringstream os;
    os << _request->getBinary().size();
    if (_method == "POST")
        content_length += os.str();
    else if (_method == "GET") {
        query_string += _request->GetQuery();
    }

    request_method += _method;

    std::vector<LocationBlock> location_vector;
    std::vector<LocationBlock>::iterator it;
    location_vector = _request->GetServer().GetLocation();
    it = location_vector.begin();
    for (; it != location_vector.end(); it++) {
        if (it->GetLocationTarget() == _target)
            break;
    }
    _target = fetchFilePath();
    // script_filename += (it->GetRoot()); //./html/post
    // script_name += (it->GetRoot());
    script_filename += _target; // index.php
    request_uri += _target;
    script_name +=
        request_uri.substr(request_uri.find_last_of("/") + 1); // index.php

    std::string _root = document_root + it->GetRoot();

    std::ostringstream os2;
    os2 << _request->GetErrorCode();
    redirect_status += os2.str();
    _envMap["CONTENT_LENGTH"] = content_length;
    // _envMap["CONTENT_TYPE"] = (char *)content_type.c_str();
    _envMap["SCRIPT_FILENAME"] = script_filename; // locationblock으ㅣ 루루트트

    _envMap["REDIRECT_STATUS"] = redirect_status;
    _envMap["SCRIPT_NAME"] = script_name;
    _envMap["REQUEST_URI"] = request_uri;
    _envMap["DOCUMENT_ROOT"] = _root;
    _envMap["QUERY_STRING"] = query_string; // get 일 때 필요
    _envMap["REQUEST_METHOD"] = request_method;
    return (_envMap);
}

void Response::forkPhp(struct kevent *curEvents,
                       std::vector<struct kevent> &_changeList) {
    std::string *body;

    if (_request->GetMethod() == "GET") {
        // body = new std::string(_request->GetTarget());
        // body->erase(0, body->find("?") + 1);
        body = new std::string(_request->GetQuery());
        // "REQUEST_METHOD=GET"
    } else if (_request->GetMethod() == "POST") {
        std::vector<char> bodyVec = _request->getBinary();
        body = new std::string(bodyVec.begin(), bodyVec.end());
    } else {
        body = new std::string();
    }
    int parentWrite[2];
    int childWrite[2];
    int pid;

    if (pipe(parentWrite) == -1 || pipe(childWrite) == -1) {
        close(parentWrite[0]);
        close(parentWrite[1]);
        delete body;
        throw ServerError(strerror(errno));
    }
    *body += "\r\n\r\n";

    std::map<std::string, std::string> _envMap = PhpEnvSet();

    const char *env[] = {
        "GATEWAY_INTERFACE=CGI/1.1",
        "SERVER_PROTOCOL=HTTP/1.1", /// gateway, protocol is const!
        "CONTENT_TYPE=application/x-www-form-urlencoded", // const same!
        (char *)_envMap["REQUEST_METHOD"].c_str(),
        (char *)_envMap["CONTENT_LENGTH"].c_str(),
        // _envMap["CONTENT_TYPE"].c_str() ,
        (char *)_envMap["SCRIPT_FILENAME"].c_str(),
        (char *)_envMap["REDIRECT_STATUS"].c_str(),
        (char *)_envMap["QUERY_STRING"].c_str(), // post이면 의미없는 문자열일터
        // "REDIRECT_STATUS=200",
        (char *)_envMap["SCRIPT_NAME"].c_str(),
        (char *)_envMap["REQUEST_URI"].c_str(),
        (char *)_envMap["DOCUMENT_ROOT"].c_str(), NULL};

    pid = fork();
    // char *env[];

    if (pid == 0) // child
    {
        close(parentWrite[1]);
        close(childWrite[0]);
        dup2(parentWrite[0], 0);
        close(parentWrite[0]);
        dup2(childWrite[1], 1);
        close(childWrite[1]);
        std::string arg2 = fetchFilePath();

        const char *arg[] = {"./html/php-cgi", (char *)arg2.c_str(), NULL};

        execve(arg[0], const_cast<char* const*>(arg), const_cast<char* const*>(env));
        exit(-1);

    } else if (pid == -1) {
        delete body;
        close(parentWrite[0]);
        close(childWrite[1]);
        close(parentWrite[1]);
        close(childWrite[0]);
        throw ServerError(strerror(errno));
    } else // Parant
    {
        close(parentWrite[0]);
        close(childWrite[1]);
        _phpUdata.body = body;
        _phpUdata.clnt_sock = curEvents->ident;
        _phpUdata.pid = pid;
        _phpUdata.child_write_zero = childWrite[0];
        _phpUdata.parent_write_one = parentWrite[1];
        struct kevent tmpEvnt;
        EV_SET(&tmpEvnt, parentWrite[1], EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0,
               &_phpUdata);
        _changeList.push_back(tmpEvnt);
        std::cout << "자식 write 등록 완료" << std::endl;
        ////////////////////////////////////////////////////////////////////////
        // curEvent->ident [clnt 소켓) udata 가 반드시 포함
        
        // string *body
        // int pid (자식 프로세스 id)
        // child write[0]
        // parent write[1]
        //////////////////////////////////////////////////////////////
    }
}

bool Response::hasChildProc() { return _hasChildProc; }

std::map<int, std::vector<int> > &Response::getCGI() { return _cgi; }

std::vector<char> Response::getResponseMessage() { return _responseMessage; }
std::vector<char> Response::getStatusHeaderMessage() {
    return _statusHeaderMessage;
}
std::vector<char> Response::getBodyMessage() { return _bodyMessage; }

// setter
void Response::setResponseMessage(std::vector<char> &responseMessage) {
    _responseMessage = responseMessage;
}
void Response::setStatusHeaderMessage(std::vector<char> &statusHeaderMessage) {
    _statusHeaderMessage = statusHeaderMessage;
}
void Response::setBodyMessage(std::vector<char> &bodyMessage) {
    _bodyMessage = bodyMessage;
}

void    Response::PhpResultRead(struct kevent *curEvnts)
{
    try{
        generatePhpResponse(curEvnts);
    }
    catch (StateCode &e) {
        _request->SetErrorCode(e._errorCode);
        _request->SetErrorMessages(e.what());
        generateErrorBody();
        _contentType = "text/html";
        generateStatusLine();
        generateHeader();
        joinResponseMessage();
        _isDone = true;
    }
}


void	Response::PhpChildWrite(struct kevent *curEvnts, std::vector<struct kevent> &_changeList) {
    phpUdata *udata = static_cast<phpUdata *>(curEvnts->udata);
    std::string *body = udata->body;
    if ((write(curEvnts->ident/*curEvents->ident*/, body->c_str(), body->length())) == -1) {
        kill(udata->pid, SIGKILL);//udata에서 가져옴
        delete body;
        close(udata->child_write_zero);//udata
        close(udata->parent_write_one);//udata
        throw ServerError("php write failed");
    }
    close(udata->parent_write_one);//udata
    struct kevent tmpEvnt;
    EV_SET(&tmpEvnt, udata->pid, EVFILT_PROC, EV_ADD | EV_ENABLE, NOTE_EXIT, 0,
            0);
    _changeList.push_back(tmpEvnt);
    _cgi[udata->pid/*udata*/].push_back(udata->clnt_sock);
    _cgi[udata->pid].push_back(udata->child_write_zero);
    delete body;
    _hasChildProc = true;
}
