#pragma once
#include <string>
#include <vector>
#include <sstream>

std::vector<std::string> splitString(std::string string, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(string);
	while (std::getline(tokenStream, token, delimiter))
	{
		tokens.push_back(token);
	}
	return tokens;
}
