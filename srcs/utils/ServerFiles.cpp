#include "ServerFiles.hpp"
#include "StateCode.hpp"
#include <cerrno>
#include <fstream>
#include <sstream>
#include <vector>
#include "Response.hpp"
ServerFiles::ServerFiles() {}

ServerFiles::~ServerFiles() {}

std::map<std::string, std::vector<char> > &ServerFiles::getFile() {
	return _file;
}

std::vector<char> ServerFiles::readTextFile(std::string filename) {
	std::ifstream tmp;
	std::stringstream ss;

	tmp.open(filename);
	if (!tmp.is_open()) {
		if (errno == EACCES)
			throw PermissionDenied();
		else if (errno == ENOENT)
			throw NotExist();
	}
	tmp.seekg(0, std::ios::end);
	std::streampos tmpSize = tmp.tellg();
	tmp.seekg(0, std::ios::beg);
	std::vector<char> file(tmpSize);

	// vector로 파일 읽기
	if (!tmp.read(&file[0], tmpSize))
		throw ServerError("reading file failed\0");
	return file;
}

std::vector<char> ServerFiles::readBinaryFile(std::string filename) {
	std::ifstream tmp;
	std::stringstream ss;
	tmp.open(filename, std::ios::binary);
	if (tmp.fail()) {
		if (errno == EACCES)
			throw PermissionDenied();
		else if (errno == ENOENT)
		{
			throw NotExist();
		}
	}
	tmp.seekg(0, std::ios::end);
	std::streampos tmpSize = tmp.tellg();
	tmp.seekg(0, std::ios::beg);
	std::vector<char> file(tmpSize);

	if (!tmp.read(&file[0], tmpSize))
		throw ServerError("reading file failed");
	return file;
}

std::vector<char> &ServerFiles::getFile(std::string filename) {
	std::map<std::string, std::vector<char> >::iterator iter = _file.find(filename);
	if (iter != _file.end())
		return iter->second;
	if (!filename.substr(static_cast<int>(filename.size()) - 4 >= -1 ? filename.size() - 4 : -1).compare(".png"))
	{
		_file.insert(std::pair<std::string, std::vector<char> >(
			filename, readBinaryFile(filename)));

	}
	else
	{
		_file.insert(std::pair<std::string, std::vector<char> >(
			filename, readTextFile(filename)));

	}
	iter = _file.find(filename);
	return iter->second;
}

void	ServerFiles::saveBinaryFile(std::string	filename, std::vector<char>	fileContent) {
	std::ofstream filePath(filename.c_str(), std::ios::out | std::ios::binary);

	if (!filePath.is_open()) {
		throw ServerError(strerror(errno));
	}
	filePath.write(&fileContent[0], fileContent.size());
	filePath.close();
}

void	ServerFiles::saveTextFile(std::string filename, std::vector<char> fileContent) {
	 std::ofstream filePath(filename);

	if (!filePath.is_open()) {
		throw ServerError(strerror(errno));
	}
	filePath.write(&fileContent[0], fileContent.size());
	filePath.close();
}

void ServerFiles::saveFile(std::string filename, std::vector<char> fileContent, std::string	contentType) {

	if (!filename.substr(static_cast<int>(filename.size()) - 4 >= -1 ? filename.size() - 4 : -1).compare(".png"))
		saveBinaryFile(filename, fileContent);
	else if (contentType == "image/png")
		saveBinaryFile(filename + ".png", fileContent);
	else
		saveTextFile(filename, fileContent);
}

void	ServerFiles::deleteFile(std::string	filename) {
	_file.erase(filename);
}
