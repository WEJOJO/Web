#include "Php.hpp"
#include "Response.hpp"
#include "get_next_line.h"

bool IsPhp(Request *reque) {
    if (reque->GetMethod() == "DELETE")
        return false;
    std::string target = reque->GetTarget();
    // get일때 본문이 있으면 url 로 들어와서 짤라줌
    if (reque->GetMethod() == "GET" && target.find('?') != std::string::npos)
        target.erase(target.find('?'), target.length()); // length 실험중
    if (target.find(".php") == target.length() - 4) {
        size_t size = reque->GetServer().GetLocation().size();
        for (size_t i = 0; i < size; i++) {
            std::string locationTarget =
                reque->GetServer().GetLocation()[i].GetLocationTarget();
            if (!strncmp(locationTarget.c_str(), target.c_str(),
                         locationTarget.size()))
                return (true);
        }
    }
    return false;
}