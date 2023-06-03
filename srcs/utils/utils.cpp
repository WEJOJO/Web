#include <cstring>
#include <string>
#include <vector>

std::vector<std::string> Split(std::string lines, std::string charSet) {
	char *token;
	char *rest = const_cast<char *>(lines.c_str());
	std::vector<std::string> splited;

	while ((token = strtok_r(rest, charSet.c_str(), &rest)))
		splited.push_back(std::string(token));
	return splited;
}

std::vector<std::string> Split2(std::string lines, std::string charSet) {
	std::string::size_type startPos = 0;
	std::string::size_type charSetPos = 0;
	std::vector<std::string> splited;

	charSetPos = lines.find(charSet, startPos);
	while (charSetPos != std::string::npos) {
		splited.push_back(lines.substr(startPos, charSetPos - startPos));
		startPos = charSetPos + charSet.length();
		if (startPos < lines.length())
			charSetPos = lines.find(charSet, startPos);
		else
			break;
	}
	if (startPos < lines.length())
		splited.push_back(lines.substr(startPos));
	return splited;
}
