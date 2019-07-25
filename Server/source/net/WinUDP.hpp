#pragma once
#include "../interface/UDPServer.hpp"
#include <winsock2.h>

class WinUDP : public UDPServer
{
public:
	SOCKET listening;

	void onStart() override
	{
		WSAData wsData;
		if (WSAStartup(0x202, &wsData))
		{
			throw std::runtime_error("WinSock could not be started: " + std::to_string(WSAGetLastError()));
		}

		listening = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		if (listening == INVALID_SOCKET)
		{
			throw std::runtime_error("Socket could not be created: " + std::to_string(WSAGetLastError()));
		}

		SOCKADDR_IN local;
		local.sin_family = AF_INET;
		local.sin_port = htons(4158);
		local.sin_addr.s_addr = INADDR_ANY;
		if (bind(listening, (SOCKADDR*)&local, sizeof(local)) == SOCKET_ERROR)
		{
			throw std::runtime_error("Failed to bind port: " + std::to_string(WSAGetLastError()));
		}
	};

	void onTick() override
	{
		
	};

	void onShutdown() override
	{
		WSACleanup();
	};
};