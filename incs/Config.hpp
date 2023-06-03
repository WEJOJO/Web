#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "ServerBlock.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>


//struct ServerBlock {
//    int _Port;
//    std::vector<std::string> _ServerName;
//    std::map<int, std::string> _ErrorPage;
//    unsigned long _ClientMaxBodySize;
//    std::string _Root;
//    std::vector<std::string> _Index;
//    std::vector<LocationBlock> _Location;
//    int _LocationBlockSize;
//    ServerBlock() {
//        _Port = -1;
//        _ClientMaxBodySize = 1024;
//        _Root = "";
//        _LocationBlockSize = 0;
//    }
//};

class Config {
  private:
    Config();
    std::string OpenFile(std::string filename);

    //server
    std::string _ServerBlockWords[6];
    void  ServerWordCheck(std::stringstream& ss, std::string token);
    void  ServerListen(ServerBlock& curServerBlockObject, std::stringstream& ss);
    void  ServerServerName(ServerBlock& curServerBlockObject, std::stringstream& ss);
    void  ServerErrorPage(ServerBlock& curServerBlockObject, std::stringstream& ss);
    void  ServerClineMaxBodySize(ServerBlock& curServerBlockObject, std::stringstream& ss);
    void  ServerRoot(ServerBlock& curServerBlockObject, std::stringstream& ss);
    void  ServerIndex(ServerBlock& curServerBlockObject, std::stringstream& ss);
    //location
    std::string _LocationBlockWords[6];
    void  LocationWordCheck(LocationBlock& curLocationBlock, std::stringstream& ss);
    void  LocationRoot(LocationBlock& curLocationBlock, std::stringstream& ss);
    void  LocationLimitExcept(LocationBlock& curLocationBlock, std::stringstream& ss);
    void  LocationAutoIndex(LocationBlock& curLocationBlock, std::stringstream& ss);
    void  LocationIndex(LocationBlock& curLocationBlock, std::stringstream& ss);
    void  LocationReturn(LocationBlock& curLocationBlock, std::stringstream& ss);
    void  LocationUploadPass(LocationBlock& curLocationBlock, std::stringstream& ss);
    void  LocationEssentialCheck(ServerBlock& curServerBlockObject, LocationBlock& curLocationBlock);

  public:
    typedef std::vector<ServerBlock>::iterator iterator;
    std::vector<ServerBlock> _ServerBlockObject;
    Config(std::string filename);
    ~Config();
    void LocationSort();
    // int		nginx_word(std::string word);
};

#endif
