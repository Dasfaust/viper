#pragma once
#include "../interface/UDP.hpp"
#include <winsock2.h>
#include <Ws2tcpip.h>

class UDPServerWin : public UDP
{
public:
	SOCKET listen;
	fd_set set;
	char buffer[4096];
	TIMEVAL timeout;

	void onStartAsync() override
	{
		timeout = { 0, 1000 };

		WSAData wsData;
		if (WSAStartup(0x202, &wsData))
		{
			crit("Server: WinSock could not be started: %d", WSAGetLastError());
			return;
		}

		listen = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (listen == INVALID_SOCKET)
		{
			crit("Server: Socket could not be created: %d", WSAGetLastError());
			return;
		}

		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(address.port);
		if (address.address.empty())
		{
			addr.sin_addr.s_addr = INADDR_ANY;
		}
		else
		{
			inet_pton(AF_INET, address.address.c_str(), &addr.sin_addr);
		}

		if (bind(listen, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
		{
			crit("Server: Failed to bind port: %d", WSAGetLastError());
			return;
		}

		FD_ZERO(&set);
		FD_SET(listen, &set);

		info("UDP server listening on port %d", address.port);
	};

	void onTickAsync() override
	{
		for (int i = 0; i < select(listen + 1, &set, nullptr, nullptr, &timeout); i++)
		{
			ZeroMemory(&buffer, 4096);

			sockaddr_in clientAddr;
			int clientSize = (int)sizeof(clientAddr);
			ZeroMemory(&clientAddr, clientSize);

			if (recvfrom(listen, buffer, 4096, 0, (sockaddr*)&clientAddr, &clientSize))
			{
				char clientIp[256];
				ZeroMemory(&clientIp, 256);
				inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, 256);
				uint32 clientPort = ntohs(clientAddr.sin_port);
				InetAddress clientAddress = { clientIp, clientPort };

				//debug("Server recv (%s:%d): %s", clientAddress.address.c_str(), clientAddress.port, buffer);

				auto packet = extract(buffer);
				if (packet.packet.empty())
				{
					warn("Invalid packet structure: %s", buffer);
					return;
				}

				std::string ai = clientAddress.address + std::to_string(clientAddress.port);
				if (!clientIds.count(ai))
				{
					clientIds[ai] = boost::uuids::random_generator()();
					clients[clientIds[ai]] = clientAddress;
				}
				packet.clients.push_back(clientIds[ai]);

				incoming.enqueue(packet);
			}
		}

		PacketWrapper<std::string> wrapper;
		while (outgoing.try_dequeue(wrapper))
		{
			if (wrapper.clients.empty())
			{
				// send to all
			}
			else
			{
				for (uid client : wrapper.clients)
				{
					if (clients.count(client))
					{
						InetAddress addr = clients[client];
						sockaddr_in clientAddr;
						clientAddr.sin_family = AF_INET;
						clientAddr.sin_port = htons(addr.port);
						inet_pton(AF_INET, addr.address.c_str(), &clientAddr.sin_addr);

						auto data = std::to_string(wrapper.id) + wrapper.packet;
						int ok = sendto(listen, data.c_str(), (uint32)data.size() + 1, 0, (sockaddr*)&clientAddr, (int)sizeof(clientAddr));
						debug("Server send (%s:%d): %d bytes, WSA: %d", addr.address.c_str(), addr.port, ok, WSAGetLastError());
					}
					else
					{
						warn("Trying to send packet to non-existing client");
					}
				}
			}
		}
	};

	void onStopAsync() override
	{
		closesocket(listen);
		WSACleanup();
	};
};