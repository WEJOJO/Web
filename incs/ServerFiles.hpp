#ifndef SERVERFILES_HPP
# define SERVERFILES_HPP

#include <map>
#include <iostream>
#include <string>
#include <exception>

class ServerFiles {
private:
	 std::map<std::string, std::vector<char> >	_file;
public:
	ServerFiles();
	~ServerFiles();

	//getter
	std::map<std::string, std::vector<char> > &getFile();

	std::vector<char>	&getFile(std::string filename);
	void				saveFile(std::string filename, std::vector<char> fileContent, std::string	contentType);
    void 				deleteFile(std::string filename);

    std::vector<char> 	readTextFile(std::string filename);
    std::vector<char> 	readBinaryFile(std::string filename);

	void				saveBinaryFile(std::string	filename, std::vector<char> fileContent);
	void				saveTextFile(std::string	filename, std::vector<char> fileContent);
};

#endif