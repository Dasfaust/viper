#pragma once
#include <string>
#include <boost/algorithm/string.hpp>
#include "../Macros.h"

namespace FileUtils
{
	inline std::string getPathSeperator();
	inline std::string getWorkingDirectory();

#ifdef V3_WIN64
#include <windows.h>
	inline std::string getPathSeperator()
	{
		return "\\";
	}

	inline std::string getWorkingDirectory()
	{
		wchar_t res[_MAX_PATH + 1];
		GetModuleFileName(NULL, res, _MAX_PATH);
		std::wstring wstr(res);
		std::string str(wstr.begin(), wstr.end());
		boost::replace_all(str, "\\Client.exe", "");
		return str;
	}
#elif defined V3_LIN64
#include <unistd.h>
#include <limits.h>
	inline std::string getPathSeperator()
	{
		return "/";
	}

	inline std::string getWorkingDirectory()
	{
		char result[PATH_MAX];
		ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
		std::string res(result, (count > 0) ? count : 0);
		boost::replace_all(res, "Client/", "");
		return res;
	}
#endif
}