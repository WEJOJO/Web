#include "ServerBlock.hpp"

ServerBlock::ServerBlock() {
    this->_Port = -1;
    this->_ClientMaxBodySize = 1024;
    this->_Root = "";
}

ServerBlock::~ServerBlock() {}

void ServerBlock::ServerBlockClear() {
    this->_Port = -1;
    this->_ClientMaxBodySize = 1024;
    this->_Root = "";
    this->_ErrorPage.clear();
    this->_ServerName.clear();
    this->_Index.clear();
    this->_Location.clear();
}

void ServerBlock::SetPort(int port) { this->_Port = port; }
void ServerBlock::SetErrorPage(std::map<int, std::string> errorpage) {
    this->_ErrorPage = errorpage;
}
void ServerBlock::SetClientMaxBodySize(unsigned long clientmaxbodysize) {
    this->_ClientMaxBodySize = clientmaxbodysize;
}
void ServerBlock::SetRoot(std::string root) { this->_Root = root; }
void ServerBlock::SetServerName(std::vector<std::string> servernames) {
    this->_ServerName = servernames;
}
void ServerBlock::SetIndex(std::vector<std::string> index) {
    this->_Index = index;
}
void ServerBlock::SetLocation(std::vector<LocationBlock> locationblock) {
    this->_Location = locationblock;
}

//////////////// Adder
void ServerBlock::AddServerName(std::string servername) {
    this->_ServerName.push_back(servername);
}
void ServerBlock::AddServerIndex(std::string index) {
    this->_Index.push_back(index);
}
void ServerBlock::AddLocationBlock(LocationBlock locationblock) {
    this->_Location.push_back(locationblock);
}

void ServerBlock::AddErrorPage(int key, std::string value) {
    this->_ErrorPage[key] = value;
}

/////////////////////////////////////////////////////////////////

/////////////Getter/////////////
int ServerBlock::GetPort() { return this->_Port; }
std::map<int, std::string> &ServerBlock::GetErrorPage() {
    return this->_ErrorPage;
}
unsigned long ServerBlock::GetClientMaxBodySize() {
    return this->_ClientMaxBodySize;
}
std::string ServerBlock::GetRoot() { return this->_Root; }
std::vector<std::string> ServerBlock::GetServerName() {
    return this->_ServerName;
}
std::vector<std::string> ServerBlock::GetIndex() { return this->_Index; }
std::vector<LocationBlock> &ServerBlock::GetLocation() {
    return this->_Location;
}
