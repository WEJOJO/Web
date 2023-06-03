#ifndef SERVERBLOCK_HPP
#define SERVERBLOCK_HPP

#include "LocationBlock.hpp"
#include <iostream>
#include <map>

class ServerBlock {
  private:
    int _Port;
    std::map<int, std::string> _ErrorPage;
    unsigned long _ClientMaxBodySize;
    std::string _Root;
    std::vector<std::string> _ServerName;
    std::vector<std::string> _Index;
    std::vector<LocationBlock> _Location;

  public:
    ServerBlock();
    ~ServerBlock();
    void ServerBlockClear();
    ///////////Setter///////////
    void SetPort(int port);
    void SetErrorPage(std::map<int, std::string> errorpage);
    void SetClientMaxBodySize(unsigned long clientmaxbodysize);
    void SetRoot(std::string root);
    void SetServerName(std::vector<std::string> servernames);
    void SetIndex(std::vector<std::string> index);
    void SetLocation(std::vector<LocationBlock> locationblock);

    ////////////////////////////
    void AddServerName(std::string servername);
    void AddServerIndex(std::string index);
    void AddLocationBlock(LocationBlock locationblock);
    void AddErrorPage(int key, std::string value);

    ///////////Getter///////////
    int GetPort();
    std::map<int, std::string> &GetErrorPage();
    unsigned long GetClientMaxBodySize();
    std::string GetRoot();
    std::vector<std::string> GetServerName();
    std::vector<std::string> GetIndex();
    std::vector<LocationBlock> &GetLocation();
    // ServerBlock &operator=(const ServerBlock &ref);
};

#endif