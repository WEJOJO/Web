#include "StateCode.hpp"

HTTPVersionError::HTTPVersionError() {
	_errorCode = HTTPVersionNotSupported;
}

MethodError::MethodError() {
	_errorCode = NotImplemented;
}

BodySizeError::BodySizeError() {
	_errorCode = PayloadTooLarge;
}

NotExist::NotExist() {
	_errorCode = NotFound;
}

PermissionDenied::PermissionDenied() {
	_errorCode = Forbidden;
}

MethodNotAllowed::MethodNotAllowed() {
	_errorCode = NotAllowed;
}

const char *HTTPVersionError::what() const throw() {
    return "HTTP Version error : The version must be HTTP 1.1";
}

const char *MethodError::what() const throw() {
    return "Not Implemented : Bad method request";
}

const char *BodySizeError::what() const throw() {
    return "Size error : Request Entity Too Large";
}

const char *NotExist::what() const throw() {
    return "NotFound : File dose not exist";
}

const char *PermissionDenied::what() const throw() {
    return "Forbidden : Permission denied";
}

const char *MethodNotAllowed::what() const throw() {
    return "Not Allowed : Method not allowed";
}

const char* ReadFail::what() const throw() {
    return "Read operation failed";
}

ServerError::ServerError(const char *condition) {
	_errorCode = InternalServerError;
    char *msg = new char[std::strlen(condition) + 25];
    strncpy(msg, "Internal Server Error : ", 25);
    strncat(msg, condition, std::strlen(condition));
    _condition = msg;
}

const char *ServerError::what() const throw() { return _condition; }

ServerError::~ServerError() throw() {
	delete [] _condition;
}