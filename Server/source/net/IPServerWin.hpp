#pragma once
#include "../interface/IPHandler.hpp"
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

class IPServerWin : public IPHandler
{
public:
	SOCKET udp;
	SOCKET tcp;
	fd_set set;
	char buffer[4096];

	void onStart() override
	{
		p0Handler = factory->registerPacket<P0Handshake>(0);
		connectEvent = viper->getModule<Events>("events")->initModule<EventHandler<ClientConnectedEvent>>("server_clientconnectedevent");
		disconnectEvent = viper->getModule<Events>("events")->initModule<EventHandler<ClientDisconnectedEvent>>("server_clientdisconnectedevent");
	};

	void onStartAsync() override
	{
		sleep = false;

		WSAData wsData;
		if (WSAStartup(0x202, &wsData))
		{
			crit("Server: WinSock could not be started: %d", WSAGetLastError());
			return;
		}

		udp = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (udp == INVALID_SOCKET)
		{
			crit("Server: UDP socket could not be created: %d", WSAGetLastError());
			return;
		}
		int timeout = 1;
		setsockopt(udp, SOL_SOCKET, SO_RCVTIMEO, (const char*)& timeout, sizeof(timeout));

		tcp = socket(AF_INET, SOCK_STREAM, 0);
		if (tcp == INVALID_SOCKET)
		{
			crit("Server: TCP socket could not be created: %d", WSAGetLastError());
			return;
		}

		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(address.udpPort);
		if (address.address.empty())
		{
			addr.sin_addr.s_addr = INADDR_ANY;
		}
		else
		{
			inet_pton(AF_INET, address.address.c_str(), &addr.sin_addr);
		}

		if (bind(udp, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
		{
			crit("Server: failed to bind UDP port: %d", WSAGetLastError());
			return;
		}
		info("UDP server listening on port %d", address.udpPort);

		addr.sin_port = htons(address.tcpPort);
		if (bind(tcp, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
		{
			crit("Server: failed to bind TCP port: %d", WSAGetLastError());
			return;
		}
		info("TCP server listening on port %d", address.tcpPort);

		listen(tcp, SOMAXCONN);

		FD_ZERO(&set);
		FD_SET(tcp, &set);
	};

	void onTickAsync() override
	{
		if (!viper->isInitialized.load() || !viper->running.load())
		{
			return;
		}

		ZeroMemory(&buffer, 4096);

		sockaddr_in clientAddr;
		int clientSize = (int)sizeof(clientAddr);
		ZeroMemory(&clientAddr, clientSize);

		if (recvfrom(udp, buffer, 4096, 0, (sockaddr*)& clientAddr, &clientSize) > 0)
		{
			char clientIp[256];
			ZeroMemory(&clientIp, 256);
			inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, 256);
			uint32 clientPort = ntohs(clientAddr.sin_port);
			std::string host = std::string(clientIp) + ":" + std::to_string(clientPort);

			//debug("Server recv UDP (%s:%d): %s", clientIp, clientPort, buffer);

			auto packet = extract(buffer);
			if (packet.packet.empty())
			{
				warn("Invalid packet structure: %s", buffer);
				return;
			}

			if (packet.id == 0)
			{
				std::stringstream ss(packet.packet);
				cereal::JSONInputArchive archive(ss);
				P0Handshake shake;
				shake.serialize(archive);

				if (tokens.count(shake.token))
				{
					uid id = tokens[shake.token];
					clients[id].address = clientIp;
					clients[id].udpPort = clientPort;

					ClientConnectedEvent ev;
					ev.id = id;
					ev.address = clients[id];
					connectEvent->fire(ev);

					shake.status = 1;
					p0Handler->enqueue(TCP, shake, { id });

					clientHosts[host] = id;
				}
				else
				{
					warn("Client with socket %d could not be identified", shake.token);
				}
			}
			else
			{
				if (clientHosts.count(host))
				{
					if (packet.packet.empty())
					{
						warn("Invalid packet structure: %s", buffer);
						return;
					}
					packet.clients.push_back(clientHosts[host]);
					incoming.enqueue(packet);
				}
				else
				{
					warn("Wrong packet ID recieved");
				}
			}
		}

		ZeroMemory(&buffer, 4096);
		fd_set copy = set;
		TIMEVAL timeout = { 0, 500 };
		int sockets = select(0, &copy, nullptr, nullptr, &timeout);
		for (int i = 0; i < sockets; i++)
		{
			SOCKET sock = copy.fd_array[i];

			if (sock == tcp)
			{
				SOCKET client = accept(tcp, nullptr, nullptr);

				FD_SET(client, &set);

				uid id = boost::lexical_cast<std::string>(boost::uuids::random_generator()());
				tokens[(uint32)client] = id;
				clients[id] = { "", 0, 0, (uint32)client };
				P0Handshake shake;
				shake.token = (uint32)client;
				shake.status = 0;
				p0Handler->enqueue(TCP, shake, { id });

				info("Initiating client connection...");
			}
			else
			{
				int in = recv(sock, buffer, 4096, 0);
				if (in > 0)
				{
					auto packet = extract(buffer);
					if (packet.packet.empty())
					{
						warn("Invalid packet structure: %s", buffer);
						return;
					}
					packet.clients.push_back(tokens[(uint32)sock]);
					incoming.enqueue(packet);
				}
				else
				{
					uid id = tokens[(uint32)sock];
					auto addr = clients[id];
					closesocket(addr.socket);
					FD_CLR(addr.socket, &set);
					ClientDisconnectedEvent ev;
					ev.id = id;
					ev.reason = "CONNECTION_CLOSED";
					disconnectEvent->fire(ev);
					clientHosts.erase(addr.address + ":" + std::to_string(addr.udpPort));
					clients.erase(id);
					tokens.erase((uint32)sock);
				}
			}
		}

		PacketWrapper<std::string> wrapper;
		while (outgoing.try_dequeue(wrapper))
		{
			if (wrapper.clients.empty())
			{
				if (wrapper.type == UDP)
				{
					for (auto&& kv : clients)
					{
						InetAddress addr = kv.second;
						sockaddr_in clientAddr;
						clientAddr.sin_family = AF_INET;
						clientAddr.sin_port = htons(addr.udpPort);
						inet_pton(AF_INET, addr.address.c_str(), &clientAddr.sin_addr);

						auto data = std::to_string(wrapper.id) + wrapper.packet;
						int ok = sendto(udp, data.c_str(), (uint32)data.size() + 1, 0, (sockaddr*)& clientAddr, (int)sizeof(clientAddr));
						//debug("Server send UDP (%s:%d): %d bytes, WSA: %d", addr.address.c_str(), addr.udpPort, ok, WSAGetLastError());
					}
				}
				else
				{
					for (uint32 i = 0; i < set.fd_count; i++)
					{
						SOCKET out = set.fd_array[i];
						if (out != tcp)
						{
							auto data = std::to_string(wrapper.id) + wrapper.packet;
							int ok = send(out, data.c_str(), (uint32)data.size() + 1, 0);
							//debug("Server send TCP (%d): %d bytes, WSA: %d", (uint32)out, ok, WSAGetLastError());
						}
					}
				}
			}
			else
			{
				for (uid client : wrapper.clients)
				{
					if (clients.count(client))
					{
						InetAddress addr = clients[client];
						if (wrapper.type == UDP)
						{
							sockaddr_in clientAddr;
							clientAddr.sin_family = AF_INET;
							clientAddr.sin_port = htons(addr.udpPort);
							inet_pton(AF_INET, addr.address.c_str(), &clientAddr.sin_addr);

							auto data = std::to_string(wrapper.id) + wrapper.packet;
							int ok = sendto(udp, data.c_str(), (uint32)data.size() + 1, 0, (sockaddr*)& clientAddr, (int)sizeof(clientAddr));
							//debug("Server send UDP (%s:%d): %d bytes, WSA: %d", addr.address.c_str(), addr.udpPort, ok, WSAGetLastError());
						}
						else
						{
							for (uint32 i = 0; i < set.fd_count; i++)
							{
								SOCKET out = set.fd_array[i];
								if (out == addr.socket)
								{
									auto data = std::to_string(wrapper.id) + wrapper.packet;
									int ok = send(out, data.c_str(), (uint32)data.size() + 1, 0);
									//debug("Server send TCP (%d): %d bytes, WSA: %d", (uint32)out, ok, WSAGetLastError());
								}
							}
						}
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
		closesocket(udp);
		closesocket(tcp);
		WSACleanup();
	};
};