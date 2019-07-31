#pragma once
#include "interface/UDP.hpp"
#include <winsock2.h>
#include <Ws2tcpip.h>

class UDPClientWin : public UDP
{
public:
	SOCKET listen;
	fd_set set;
	char buffer[4096];
	TIMEVAL timeout;
	sockaddr_in serverAddr;

	void onStartAsync() override
	{
		timeout = { 0, 1000 };

		WSAData wsData;
		if (WSAStartup(0x202, &wsData))
		{
			crit("Client: WinSock could not be started: %d", WSAGetLastError());
			return;
		}

		listen = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (listen == INVALID_SOCKET)
		{
			crit("Client: Socket could not be created: %d", WSAGetLastError());
			return;
		}
		int timeout = 1;
		setsockopt(listen, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(address.port);
		inet_pton(AF_INET, address.address.c_str(), &serverAddr.sin_addr);

		FD_ZERO(&set);
		FD_SET(listen, &set);

		info("UDP client initialized");
	};

	void onTickAsync() override
	{
		PacketWrapper<std::string> wrapper;
		while (outgoing.try_dequeue(wrapper))
		{
			if (wrapper.clients.empty())
			{
				auto data = std::to_string(wrapper.id) + wrapper.packet;
				int ok = sendto(listen, data.c_str(), (uint32)data.size() + 1, 0, (sockaddr*)&serverAddr, (int)sizeof(serverAddr));
				debug("Client send: %d bytes, WSA: %d", ok, WSAGetLastError());
			}
		}

		ZeroMemory(&buffer, 4096);

		sockaddr_in clientAddr;
		int clientSize = (int)sizeof(clientAddr);
		ZeroMemory(&clientAddr, clientSize);

		if (recvfrom(listen, buffer, 4096, 0, (sockaddr*)&clientAddr, &clientSize) > 0)
		{
			//debug("Client recv: %s", buffer);

			auto packet = extract(buffer);
			if (packet.packet.empty())
			{
				warn("Client: invalid packet structure: %s", buffer);
				return;
			}

			incoming.enqueue(packet);
		}
	};

	void onStopAsync() override
	{
		closesocket(listen);
		WSACleanup();
	};
};
