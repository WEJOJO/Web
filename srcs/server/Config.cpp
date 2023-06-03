#include "Config.hpp"

int NginxWord(std::string line);
bool test(std::string line);

Config::Config() {}

void Itos(int num, std::string &ret) {
	if (num >= 10) {
		Itos(num / 10, ret);
		ret += static_cast<char>((num % 10) + 48);
	} else
		ret += static_cast<char>(num + 48);
}

Config::Config(std::string filename) {
	// 파일 열어서 파싱하기
	std::string ConfigFile;
	ConfigFile = OpenFile(filename);
	std::string token;
	std::vector<std::string> serverStack;
	std::vector<std::string> locationStack;
	ServerBlock curServerBlockObject;

	std::string ServerBlockWords[6] = {"listen",     "server_name",
									   "error_page", "client_max_body_size",
									   "root",       "index"};
	std::string LocationBlockWords[6] = {
		"root", "limit_except", "autoindex", "index", "return", "upload_pass"};
	void (Config::*ServerFunc[6])(ServerBlock &, std::stringstream &) = {
		&Config::ServerListen,    &Config::ServerServerName,
		&Config::ServerErrorPage, &Config::ServerClineMaxBodySize,
		&Config::ServerRoot,      &Config::ServerIndex};
	void (Config::*LocationFunc[6])(LocationBlock &, std::stringstream &) = {
		&Config::LocationRoot,      &Config::LocationLimitExcept,
		&Config::LocationAutoIndex, &Config::LocationIndex,
		&Config::LocationReturn,    &Config::LocationUploadPass};

	///// ServerBlock CurrServerBlock; /////////
	std::stringstream ss(ConfigFile);
	while (ss >> token) {
		ServerWordCheck(ss, token);
		serverStack.push_back("{");
		curServerBlockObject.ServerBlockClear();
		while (!serverStack.empty() && ss >> token) {
			for (int i = 0; i < 6; i++) {
				if (token == ServerBlockWords[i]) {
					(this->*(ServerFunc[i]))(curServerBlockObject, ss);
					break;
				}
			}
			if (token == "}") {
				if (curServerBlockObject.GetLocation().size() == 0) {
					std::cout << "non 존재 LocationBlock err" << std::endl;
					exit(1);
				}
				serverStack.pop_back();
				_ServerBlockObject.push_back(curServerBlockObject);
				continue;
			}
			if (token == "location") {
				LocationBlock curLocationBlock;
				LocationWordCheck(curLocationBlock, ss);
				locationStack.push_back("{");
				while (!locationStack.empty() && ss >> token) {
					for (int i = 0; i < 6; i++) {
						if (token == LocationBlockWords[i]) {
							(this->*(LocationFunc[i]))(curLocationBlock, ss);
							break;
						}
					}
					if (token == "}") {
						LocationEssentialCheck(curServerBlockObject,
											   curLocationBlock);
						locationStack.pop_back();
						curServerBlockObject.AddLocationBlock(curLocationBlock);
					}
				}
			}
		}
	}
	if (!serverStack.empty() || !locationStack.empty()) {
		std::cout << "scope err" << std::endl;
	}
}

Config::~Config() {}

std::string Config::OpenFile(std::string filename) {
	// 파일 open
	std::ifstream file(filename);
	std::string file_Save = "";
	std::string line;
	if (!file.is_open()) {
		std::cout << "file not found" << std::endl;
		exit(1);
	}
	while (getline(file, line, '\n')) {
		if (NginxWord(line) == 0) {
			std::cout << "; err" << std::endl;
			exit(1);
		}
		file_Save += line;
		file_Save += " ";
	}
	return file_Save;
}

// serverBlock setting
void Config::ServerWordCheck(std::stringstream &ss, std::string token) {
	if (token != "server") {
		std::cout << " or must \"server\" first" << std::endl;
		exit(1);
	}
	ss >> token;
	if (token != "{") {
		std::cout << "server { 필수" << std::endl;
	}
}
void Config::ServerListen(ServerBlock &curServerBlockObject,
						  std::stringstream &ss) {
	std::string token;
	(ss >> token);
	int size = token.size() - 1;
	std::string test_len = "";
	if (curServerBlockObject.GetPort() != -1)
		return;
	Itos(atoi(token.c_str()), test_len);
	if ((int)test_len.size() == size)
		curServerBlockObject.SetPort(atoi(token.c_str()));
	else {
		std::cout << "port err" << std::endl;
		exit(1);
	}
	return;
}
void Config::ServerServerName(ServerBlock &curServerBlockObject,
							  std::stringstream &ss) {
	std::string token;
	while (ss >> token) {
		if (token[token.size() - 1] == ';') {
			token.erase(token.size() - 1, 1);
			curServerBlockObject.AddServerName(token);
			break;
		}
		curServerBlockObject.AddServerName(token);
	}
}
void Config::ServerErrorPage(ServerBlock &curServerBlockObject,
							 std::stringstream &ss) {
	std::string token;
	std::vector<std::string> temp;
	while (ss >> token) {
		if (token[0] == '/')
			break;
		temp.push_back(token);
	}
	token.erase(token.size() - 1, 1);
	for (size_t i = 0; i < temp.size(); i++) {
		curServerBlockObject.AddErrorPage(atoi(temp[i].c_str()), token);
	}
}
void Config::ServerClineMaxBodySize(ServerBlock &curServerBlockObject,
									std::stringstream &ss) {
	std::string token;
	ss >> token;
	if (curServerBlockObject.GetClientMaxBodySize() == 1024) {
		curServerBlockObject.SetClientMaxBodySize(atoi(token.c_str()));
	}
}
void Config::ServerRoot(ServerBlock &curServerBlockObject,
						std::stringstream &ss) {
	std::string token;
	ss >> token;
	if (curServerBlockObject.GetRoot().empty()) {
		token.erase(token.size() - 1, 1);
		curServerBlockObject.SetRoot(token);
	}
}
void Config::ServerIndex(ServerBlock &curServerBlockObject,
						 std::stringstream &ss) {
	std::string token;
	while (ss >> token) {
		if (token[token.size() - 1] == ';')
			break;
		curServerBlockObject.AddServerIndex(token);
	}
	token.erase(token.size() - 1, 1);
	curServerBlockObject.AddServerIndex(token);
}
// locationBlock setting
void Config::LocationWordCheck(LocationBlock &curLocationBlock,
							   std::stringstream &ss) {
	std::string token;
	ss >> token;
	curLocationBlock.SetLocationTarget(token);
	ss >> token;
	if (token != "{") {
		std::cout << "\"{\" must appear after location target" << std::endl;
		exit(1);
	}
}
void Config::LocationRoot(LocationBlock &curLocationBlock,
						  std::stringstream &ss) {
	std::string token;
	(ss >> token);
	if (curLocationBlock.GetRoot().empty()) {
		token.erase(token.size() - 1, 1);
		curLocationBlock.SetRoot(token);
	}
}
void Config::LocationLimitExcept(LocationBlock &curLocationBlock,
								 std::stringstream &ss) {
	std::string token;
	int arr[3] = {0, };
	int size = 0;
	while (ss >> token) {
		if (token == "{")
			break;
		if ((token == "GET" || token == "POST" || token == "DELETE") &&
			size < 3) {
			if (token == "GET") {
				if (arr[0] == 1) {
					std::cout << "limit_except err" << std::endl;
					exit(1);
				}
				arr[0] = 1;
			} else if (token == "POST") {
				if (arr[1] == 1) {
					std::cout << "limit_except err" << std::endl;
					exit(1);
				}
				arr[1] = 1;
			} else if (token == "DELETE") {
				if (arr[2] == 1) {
					std::cout << "limit_except err" << std::endl;
					exit(1);
				}
				arr[2] = 1;
			}
			size++;
			curLocationBlock.AddLimitExcept(token);
		} else {
			std::cout << "limit_except err" << std::endl;
			exit(1);
		}
	}
	if (size == 0) {
		std::cout << "limit_except err" << std::endl;
		exit(1);
	}
	ss >> token;
	if (token != "deny") {
		std::cout << "limit_except err" << std::endl;
		exit(1);
	}
	ss >> token;
	if (token != "all;") {
		std::cout << "limit_except err" << std::endl;
		exit(1);
	}
	ss >> token;
	if (token != "}") {
		std::cout << "limit_except err" << std::endl;
		exit(1);
	}
}
void Config::LocationAutoIndex(LocationBlock &curLocationBlock,
							   std::stringstream &ss) {
	std::string token;
	ss >> token;
	if (curLocationBlock.GetAutoIndex() == -1) {
		if (token == "on;") {
			curLocationBlock.SetAutoIndex(1);
		} else if (token == "off;") {
			curLocationBlock.SetAutoIndex(0);
		} else {
			std::cout << "autoindex err" << std::endl;
			exit(1);
		}
	}
}
void Config::LocationIndex(LocationBlock &curLocationBlock,
						   std::stringstream &ss) {
	std::string token;
	while (ss >> token) {
		if (token[token.size() - 1] == ';')
			break;
		curLocationBlock.AddIndex(token);
	}
	token.erase(token.size() - 1, 1);
	curLocationBlock.AddIndex(token);
}
void Config::LocationReturn(LocationBlock &curLocationBlock,
							std::stringstream &ss) {
	std::string token;
	if (curLocationBlock.GetReturn().first == 0 &&
		curLocationBlock.GetReturn().second == "") {
		ss >> token;
		std::pair<int, std::string> temp;
		temp.first = atoi(token.c_str());
		ss >> token;
		token.erase(token.size() - 1, 1);
		temp.second = token;
		curLocationBlock.SetReturn(temp);
	} else {
		while (ss >> token) {
			if (token[token.size() - 1] == ';')
				break;
		}
	}
}
void Config::LocationUploadPass(LocationBlock &curLocationBlock,
								std::stringstream &ss) {
	std::string token;
	if (curLocationBlock.GetUploadPass() != "") {
		return;
	}
	ss >> token;
	token.erase(token.size() - 1, 1);
	curLocationBlock.SetUploadPass(token);
}

void Config::LocationEssentialCheck(ServerBlock &curServerBlockObject,
									LocationBlock &curLocationBlock) {
	if (curLocationBlock.GetRoot() == "")
		curLocationBlock.SetRoot(curServerBlockObject.GetRoot());
	if (curLocationBlock.GetIndex().empty()) {
		curLocationBlock.SetIndex(curServerBlockObject.GetIndex());
	}
	if (curLocationBlock.GetRoot() == "" ||
		curLocationBlock.GetIndex().empty()) {
		std::cout << "err server root, location root or _Index" << std::endl;
		exit(1);
	}
}

bool test(std::string line) {
	size_t len = line.size() - 1;
	size_t idx = 0;
	while (line[idx] != ';') {
		idx++;
		if (len < idx)
			return false;
	}
	if (len != idx)
		return false;
	return true;
}
int NginxWord(std::string line) {
	std::stringstream ss(line);
	std::string FirstToken;
	ss >> FirstToken;
	std::string words[8] = {
		"listen", "server_name", "error_page", "client_max_body_size",
		"return", "root",        "autoindex",  "index"};
	for (int i = 0; i < 8; i++) {
		if (words[i] == FirstToken) {
			if (test(line))
				return 1;
			else {
				// std::cout << "=====semicolon test======" << std::endl;
				// std::cout << FirstToken << std::endl;
				// std::cout << "=====semicolon test======" << std::endl;

				return 0;
			}
		}
	}
	return 1;
}

bool Fcomp(LocationBlock t1, LocationBlock t2) {
    if (t1.GetLocationTarget().size() >= t2.GetLocationTarget().size())
        return true;
    else
        return false;
}

void Config::LocationSort() {
	std::vector<ServerBlock>::iterator it;
    std::vector<ServerBlock>::iterator its;
    its = _ServerBlockObject.end();
    it = _ServerBlockObject.begin();
    for (; it != its; it++) {
        std::vector<LocationBlock> vec_loc = it->GetLocation();
        std::stable_sort(vec_loc.begin(), vec_loc.end(), Fcomp);
    }
}
