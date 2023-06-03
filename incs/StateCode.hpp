#ifndef STATECODE_HPP
# define STATECODE_HPP

# include <exception>
# include <string>

enum ErrorCode {

    // 200 ~
    OK = 200,
    Created = 201,
	NoContent = 204,

    // 300 ~

    // 400 ~
    BadRequest = 400,
    Forbidden = 403,	// 접근 권한 없음
    NotFound = 404,
	NotAllowed = 405,	// 해당 메서드 사용 못함
    PayloadTooLarge = 413,

    // 500 ~
	InternalServerError = 500,
    NotImplemented = 501,
    BadGateway = 502,
    GatewayTimeout = 504,
    HTTPVersionNotSupported = 505,
};

class StateCode : public std::exception {

public:
	int	_errorCode;
	const char	*what() const throw() = 0;
};

class HTTPVersionError : public StateCode {
public:
	HTTPVersionError();
	const char *what() const throw();
};

class MethodError : public StateCode {
public:
	MethodError();
	const char *what() const throw();
};

class BodySizeError : public StateCode {
public:
	BodySizeError();
	const char *what() const throw();
};

class NotExist : public StateCode {
public:
	NotExist();
	const char *what() const throw();
};

class PermissionDenied : public StateCode {
public:
	PermissionDenied();
	const char *what() const throw();
};

class MethodNotAllowed : public StateCode {
public :
	MethodNotAllowed();
	const char *what() const throw();
};

class ServerError : public StateCode {
private :
	const char *	_condition;
public:
	ServerError(const char * condition);
	~ServerError() throw();
	virtual const char *what() const throw();
};

class ReadFail : public std::exception {
public:
	const char *what() const throw();
};

#endif