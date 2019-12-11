#pragma once
#include <string>
#include <regex>
#include <fstream>
#include "stb_image.h"
  
inline std::string seperator();
inline std::string workingDir();

#ifdef VIPER_WIN64
#include <windows.h>
inline std::string seperator()
{
	return "\\";
};

inline std::string workingDir()
{
	std::string dir;
	dir.resize(MAX_PATH);
	GetModuleFileNameA(NULL, dir.data(), MAX_PATH);
	return dir.substr(0, dir.find_last_of("\\"));
};
#elif defined VIPER_LIN64
#include <unistd.h>
#include <limits.h>
inline std::string seperator()
{
	return "/";
};

inline std::string workingDir()
{
	char result[PATH_MAX];
	ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
	std::string res(result, (count > 0) ? count : 0);
	boost::replace_all(res, "Client/", "");
	return res;
};
#endif

inline std::vector<unsigned char> readFile(const std::string& name)
{
	std::string path = workingDir() + seperator() + "resources" + seperator() + name;
	std::ifstream input(path, std::ios::binary);
	std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});
	return buffer;
};

struct ImageResource
{
	int width;
	int height;
	int channels;
	unsigned char* data = nullptr;
};

inline ImageResource readImage(const std::string& name)
{
	std::string path = workingDir() + seperator() + "resources" + seperator() + name;
	ImageResource resource;
	resource.data = stbi_load(path.c_str(), &resource.width, &resource.height, &resource.channels, 0);
	return resource;
};