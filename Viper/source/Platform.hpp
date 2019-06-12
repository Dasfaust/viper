#pragma once
#include "Viper.hpp"

namespace viper
{
	namespace platform
	{
		static void setTerminateHandler();
		static void pauseSystem();

#ifdef VIPER_WIN64
#include <windows.h>

		BOOL WINAPI CtrlHandler(DWORD type)
		{
			switch(type)
			{
				case CTRL_C_EVENT:
					Viper::shutdown();
					return TRUE;
				case CTRL_CLOSE_EVENT:
					Viper::shutdown();
					return TRUE;
				case CTRL_BREAK_EVENT:
					Viper::shutdown();
					return TRUE;
				default:
					return FALSE;
			}
		};

		inline void setTerminateHandler()
		{
			SetConsoleCtrlHandler(CtrlHandler, TRUE);
		};

		inline void pauseSystem()
		{
			system("pause");
		};

#endif
	};
};