#pragma once
#include <vector>
#include <string>
#include <sstream>

inline std::vector<std::string> splitStr(std::string& string, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(string);
	while (std::getline(tokenStream, token, delimiter))
	{
		tokens.push_back(token);
	}
	return tokens;
};

inline bool isNumber(std::string& string)
{
	return !string.empty() && string.find_first_not_of("0123456789.-") == std::string::npos;
};