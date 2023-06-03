#include "LocationBlock.hpp"
#include <iostream> //

LocationBlock::LocationBlock() {
    this->_UploadPass = "";
    this->_LocationTarget = "";
    this->_Root = "";
    this->_AutoIndex = -1;
}

LocationBlock::~LocationBlock() {}

void LocationBlock::SetLocationTarget(std::string locationtarget) {
    this->_LocationTarget = locationtarget;
}

void LocationBlock::SetUploadPass(std::string uploadpass) {
    this->_UploadPass = uploadpass;
}

void LocationBlock::SetReturn(std::pair<int, std::string> ret) {
    this->_Return = ret;
}

void LocationBlock::SetAutoIndex(int autoindex) {
    this->_AutoIndex = autoindex;
}

//// vector
void LocationBlock::AddLimitExcept(std::string limitexcept) {
    this->_LimitExcept.push_back(limitexcept);
}

void LocationBlock::AddIndex(std::string index) {
    this->_Index.push_back(index);
}

void LocationBlock::SetLimitExcept(std::vector<std::string> limitexcept) {
    this->_LimitExcept = limitexcept;
}
void LocationBlock::SetIndex(std::vector<std::string> index) {
    this->_Index = index;
}
void LocationBlock::SetRoot(std::string root) { this->_Root = root; }
////

////////////////////Getter/////////////
std::string LocationBlock::GetLocationTarget() const {
    return this->_LocationTarget;
}
std::string LocationBlock::GetRoot() const { return this->_Root; }
std::string LocationBlock::GetUploadPass() const { return this->_UploadPass; }
std::pair<int, std::string> LocationBlock::GetReturn() const {
    return this->_Return;
}
std::vector<std::string> LocationBlock::GetLimitExcept() const {
    return this->_LimitExcept;
}
std::vector<std::string> LocationBlock::GetIndex() const {
    return this->_Index;
}
int LocationBlock::GetAutoIndex() const { return this->_AutoIndex; }

LocationBlock &LocationBlock::operator=(const LocationBlock &ref) {
    this->_LocationTarget = ref.GetLocationTarget();
    this->_Root = ref.GetRoot();
    this->_UploadPass = ref.GetUploadPass();
    this->_Return = ref.GetReturn();
    this->_LimitExcept = ref.GetLimitExcept();
    this->_Index = ref.GetIndex();
    this->_AutoIndex = ref.GetAutoIndex();

    return *this;
}
