#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "ServerBlock.hpp"
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

// test를 위해 crlf를 아래와 같이 정의했습니다. 이후에 "\r\n"으로 수정 바랍니다.
#ifndef CRLF
#define CRLF "\r\n"
#endif

#ifndef CRLF_SIZE
#define CRLF_SIZE 2
#endif

class ServerBlock;

class Request {
  private:
    std::string _method;
    std::string _target;
    std::string _contentType;
    std::string _contentDisposition;
    int _contentLength;
    std::string _fileName;
    std::string _errorMessages;
    int _errorCode;
    bool _chunked;
    std::stringstream _stream;
    ServerBlock *_server;
    std::string _hostName;
    int _hostPort;
    bool _isEnd;
    std::vector<char> _binary;
    std::string _boundary;
    std::vector<Request *> _formData;
    std::string _query;
	bool	_connection;
  public:
    Request();
    Request(int fd, std::stringstream &stream);
    Request(std::vector<char> &body,
            std::string delimeter); // body에서 저장한 값을 지워주기
    Request(const Request &request);
    Request &operator=(const Request &request);
    virtual ~Request();

    // Setter
    void SetTarget(std::string target);
    void SetMethod(std::string method);
    void SetContentType(std::string contentType);
    void SetChunked(bool chunked);
    void SetStream(std::stringstream &stream);
    void SetErrorCode(int _errorCode);
    void SetErrorMessages(std::string _errorMessages);
    void SetServer(ServerBlock *serverBlock);
    void SetHostName(std::string hostName);
    void SetHostPort(int hostPort);
    void SetIsEnd(bool isEnd);
    void SetBinary(std::vector<char> &binary);
    void setStartLine(std::string startLine);
    void setHeader(std::string header);
    void setFileName(std::string fileName);

    // Getter
    std::string GetMethod();
    std::string GetTarget();
    std::string GetContentType();
    bool GetChunked();
    std::stringstream &GetStream();
    ServerBlock &GetServer();
    int GetErrorCode();
    std::string GetErrorMessages();
    std::string GetHostName();
    int GetHostPort();
    bool GetIsEnd();
    std::vector<char> getBinary();
    std::vector<Request *> getFormData();
    std::string getFileName();
	bool	getConnection();

    // void SetBody(std::vector<std::string>::iterator iter);
    void splitHost();
    void readBody(int fd);
	std::string GetQuery();
    void parseFormData();
};

Request *ParseRequest(int fd, std::map<int, Request *> &clients,
                      std::vector<ServerBlock> &servers);

#endif