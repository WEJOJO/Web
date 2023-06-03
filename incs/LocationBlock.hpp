#ifndef LOCATIONBLOCK_HPP
#define LOCATIONBLOCK_HPP

#include <string>
#include <utility>
#include <vector>

class LocationBlock {
  private:
    std::string _LocationTarget;
    std::string _Root;
    std::string _UploadPass;
    std::pair<int, std::string> _Return;
    std::vector<std::string> _LimitExcept;
    std::vector<std::string> _Index;
    int _AutoIndex;
    // LocationBlock(const LocationBlock &ref);

  public:
    LocationBlock();
    ~LocationBlock();

    /////// Setter /////////////
    void SetLocationTarget(std::string locationtarget);
    void SetRoot(std::string root);
    void SetUploadPass(std::string uploadpass);
    void SetReturn(std::pair<int, std::string> _return);
    void SetAutoIndex(int autoindex);

    ////
    void AddLimitExcept(std::string limitexcept);
    void AddIndex(std::string index);
    /////
    void SetLimitExcept(std::vector<std::string> limitexcept);
    void SetIndex(std::vector<std::string> index);
    ////////////////////////////

    /////// Getter /////////////
    std::string GetLocationTarget() const;
    std::string GetRoot() const;
    std::string GetUploadPass() const;
    std::pair<int, std::string> GetReturn() const;
    int GetAutoIndex() const;

    std::vector<std::string> AddLimitExcept();
    std::vector<std::string> AddIndex();
    ////////////////////////////

    std::vector<std::string> GetLimitExcept() const;
    std::vector<std::string> GetIndex() const;
    ////////////////////////////

    LocationBlock &operator=(const LocationBlock &ref);
};

#endif
