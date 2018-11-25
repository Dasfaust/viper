#pragma once

#ifdef V3_WIN64
	#ifdef V3_WIN64_DLL
		#define V3API __declspec(dllexport)
	#else
		#define V3API __declspec(dllimport)
	#endif
#else
	#define V3API
#endif