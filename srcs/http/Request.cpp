#include "Request.hpp"
#include "ServerBlock.hpp"
#include "StateCode.hpp"
#include "get_next_line.h"
#include "utils.hpp"
#include <algorithm>
#include <cstdlib>
#include <sys/socket.h>

Request::Request() : _chunked(false) {}

Request::Request(int fd, std::stringstream &stream)
    : _contentLength(-1), _errorMessages("OK"), _errorCode(OK), _chunked(false),
      _hostPort(80), _isEnd(false), _connection(false) {

    try {
        std::string buff;
        std::getline(stream, buff, '\n');
        setHeader(stream.str());
        splitHost();
        setStartLine(buff);
        if (GetMethod() == "POST") {
            readBody(fd);
        }
        if (_contentType == "multipart/form-data")
            parseFormData();
    } catch (ReadFail &e){
		SetErrorCode(600);
	} catch (const StateCode &e) {
        SetErrorCode(e._errorCode);
        SetErrorMessages(e.what());
    }
}

// formData를 위한 생성자
Request::Request(std::vector<char> &formData, std::string delimeter) {
    // boundary, header, body 모두 복사
    std::string headerStr(formData.begin(), formData.end());
    std::string doubleCRLF = CRLF;
    doubleCRLF.append(CRLF);
    size_t crlfPos = headerStr.find("\r\n-");
    size_t delimeterPos = headerStr.find(delimeter);

    if (delimeterPos == std::string::npos)
        return;
    setHeader(headerStr.substr(0, delimeterPos));
    if (headerStr.find(doubleCRLF) == std::string::npos)
        return;
    delimeterPos = headerStr.find(doubleCRLF) + doubleCRLF.size();
    crlfPos -= delimeterPos;
    formData.erase(formData.begin(), formData.begin() +
                                         headerStr.find(doubleCRLF) +
                                         doubleCRLF.size());
    std::copy(formData.begin(), formData.begin() + crlfPos,
              std::back_inserter(_binary));
    formData.erase(formData.begin(), formData.begin() + crlfPos);
    if (formData.size() >= 2 && formData[0] == '\r' && formData[1] == '\n')
        formData.erase(formData.begin(), formData.begin() + 2);
}

Request::~Request() {
    if (_contentType == "multipart/form-data") {
        for (size_t i = 0; i < _formData.size(); i++)
            delete _formData[i];
    }
}

void Request::SetMethod(std::string method) { _method = method; }

std::string Request::GetMethod() { return _method; }

void Request::SetTarget(std::string target) { _target = target; }

std::string Request::GetTarget() { return _target; }

void Request::SetContentType(std::string contentType) {
    _contentType = contentType;
}

std::string Request::GetContentType() { return _contentType; }

void Request::SetChunked(bool chunked) { _chunked = chunked; }

bool Request::GetChunked() { return _chunked; }

void Request::SetStream(std::stringstream &stream) {
    _stream.clear();
    _stream << stream.str();
}

std::stringstream &Request::GetStream() { return _stream; }

ServerBlock &Request::GetServer() { return *_server; }

void Request::SetErrorCode(int errorCode) { _errorCode = errorCode; }

int Request::GetErrorCode() { return _errorCode; }

void Request::SetErrorMessages(std::string errorMessages) {
    _errorMessages = errorMessages;
}

std::string Request::GetErrorMessages() { return _errorMessages; }

void Request::SetServer(ServerBlock *serverBlock) { _server = serverBlock; }
bool Request::GetIsEnd() { return _isEnd; }

void Request::SetIsEnd(bool isEnd) { _isEnd = isEnd; }

void Request::setFileName(std::string fileName) { _fileName = fileName; }

std::string Request::getFileName() { return _fileName; }

std::vector<Request *> Request::getFormData() { return _formData; }

std::vector<char> Request::getBinary() { return _binary; }

bool Request::getConnection() { return _connection; }

Request &Request::operator=(const Request &request) {
    _method = request._method;
    _target = request._target;
    _contentType = request._contentType;
    _chunked = request._chunked;
    _stream.clear();
    _stream << request._stream.str();
    return *this;
}

Request::Request(const Request &request)
    : _chunked(false), _server(request._server) {
    if (this == &request)
        return;
    *this = request;
}

void Request::setStartLine(std::string startLine) {
    std::vector<std::string> data = Split(startLine, std::string(" "));
    if (!data[0].compare("GET") || !data[0].compare("POST") ||
        !data[0].compare("DELETE")) {
        _method = data[0];
    } else {
        throw MethodError();
    }
    _target = Split(
        data[1], std::string(":"))[0]; //[0]은 vector 0번째 인덱스 접근을 위함
    if (_target.find('?') != std::string::npos) {
        _query = _target.substr(_target.find('?') + 1);
        _target.erase(_target.find('?'));
    }

    if (data[2].compare("HTTP/1.1\r"))
        throw HTTPVersionError();
}

void Request::setHeader(std::string header) {
    std::vector<std::string> splited = Split(header, " \r\n;");
    std::vector<std::string>::iterator iter;
    for (std::vector<std::string>::iterator i = splited.begin();
         i != splited.end(); i++) {
    }

    iter = find(splited.begin(), splited.end(), "Content-Type:");
    if (iter != splited.end() && iter + 1 != splited.end()) {
        _contentType = *(iter + 1);
        if (_contentType == "multipart/form-data")
            _boundary = (*(iter + 2)).substr(9);
    }
    iter = find(splited.begin(), splited.end(), "Host:");
    if (iter != splited.end() && iter + 1 != splited.end())
        _hostName = *(iter + 1); // example.com:8080
    iter = find(splited.begin(), splited.end(), "Transfer-Encoding:");
    if (iter != splited.end() && iter + 1 != splited.end() &&
        !(iter + 1)->compare("chunked"))
        _chunked = true;
    iter = find(splited.begin(), splited.end(), "Content-Length:");
    if (iter != splited.end() && iter + 1 != splited.end())
        _contentLength = atoi((iter + 1)->c_str());
    iter = find(splited.begin(), splited.end(), "Content-Disposition:");
    if (iter != splited.end()) {
        for (int i = 0; iter + i < splited.end(); i++) {
            if ((iter + i)->find("filename=") != std::string::npos) {
                _fileName = (iter + i)->substr(std::string("filename=").size());
                _fileName.erase(0, 1);
                _fileName.erase(_fileName.size() - 1, 1);
            }
        }
    }
    iter = find(splited.begin(), splited.end(), "Connection:");
    if (iter != splited.end() && iter + 1 != splited.end())
        _connection = (*(iter + 1) == "keep-alive" ? true : false);
}

void Request::splitHost() {
	if (_hostName.find(":") != std::string::npos && _hostName.find(":") != _hostName.size()) {
		_hostPort = atoi(_hostName.substr(_hostName.find(":") + 1).c_str());
	}
	_hostName = _hostName.substr(0, _hostName.find(":"));
}

std::string Request::GetHostName() { return this->_hostName; }

int Request::GetHostPort() { return this->_hostPort; }

void Request::readBody(int fd) {
    unsigned int x = 0;
    std::stringstream ss;
    std::string line;
    int valRead;
    if (_chunked) {
        char *str = get_next_line(fd);
        line = str;
        free(str);
        line = line.substr(0, line.size() - CRLF_SIZE);
        ss << std::hex << line;
        ss >> x;
        _contentLength = x;
        // std::cout << "chunked = " << x << std::endl;
        if (x == 0) {
            _isEnd = true;
            return;
        }
    }
    // std::cout << _contentLength << std::endl;
    if (_contentLength == -1) // contentLength 가 없고 chunked가 아닌데 바디가 있는 경우
        _contentLength = 10;
    std::vector<char> buffer(_contentLength);
    valRead = recv(fd, &buffer[0], _contentLength, 0); //non block * block
    if (valRead == _contentLength) {
        // std::cout << "valread good!!!" << std::endl;
        std::copy(buffer.begin(), buffer.begin() + valRead,
                std::back_inserter(_binary));
    }
    else {
        // std::cout << "valread == -1" << std::endl;
        throw ReadFail();
    }



    // CRLF가 2번인지 확인해서 다 읽었는지 아닌지 확인
    if (!_chunked && _binary.size()) {
        size_t end = _binary.size() - 1;
        if (end > 3 && _binary[end] == '\n' && _binary[end - 1] == '\r' &&
            _binary[end - 2] == '\n' && _binary[end - 3] == '\r') {
            _isEnd = true;
            _binary.erase(_binary.end() - 5, _binary.end() - 1);
        }
    } else if (_chunked) {
        if (static_cast<unsigned int>(valRead) > x ||
            (_server != NULL &&
             _binary.size() > _server->GetClientMaxBodySize())) {  //chunked 일때 _binary에 이어붙이고 있는데
             std::string str(_binary.begin(), _binary.end());      // 조건문을 보면 chunked여도 maxbodysize 못넘길걸로 예상
            throw BodySizeError();
             }
        _isEnd = false;
    } else if (_binary.size() == 0)
        _isEnd = true;
}

void Request::parseFormData() {
    size_t i = 0;
    while (_boundary[i] == '-')
        i++;
    _boundary.erase(0, i);
    i = 0;
    while (1) {
        i = 0;
        while (i < _binary.size() && _binary[i] == '-')
            i++;
        _binary.erase(_binary.begin(), _binary.begin() + i);
        i = 0;
        while (i < _binary.size() && _binary[i] == _boundary[i])
            i++;
        _binary.erase(_binary.begin(), _binary.begin() + i);
        i = 0;
        if (_binary.size() >= 2 && _binary[i] == '\r' && _binary[i + 1] == '\n')
            _binary.erase(_binary.begin(), _binary.begin() + 2);
        if (_binary.size() == 0 || _binary[0] == '-')
            break;
        _formData.push_back(new Request(_binary, _boundary));
    }
}

std::string Request::GetQuery() { return _query; }
