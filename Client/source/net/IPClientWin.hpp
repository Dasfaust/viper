#pragma once
#include "interface/IPHandler.hpp"
#include <winsock2.h>
#include <Ws2tcpip.h>

class IPClientWin : public IPHandler, public std::enable_shared_from_this<IPClientWin>
{
public:
	SOCKET udp;
	SOCKET tcp;
	fd_set set;
	char buffer[4096];
	sockaddr_in udpAddr;
	sockaddr_in tcpAddr;
	std::atomic_bool connected = false;
	std::shared_ptr<Listener<P0Handshake>> p0Listener;

	void onStart() override
	{
		p0Handler = factory->registerPacket<P0Handshake>(0);
		p0Listener = p0Handler->listen(0, [](P0Handshake& packet, std::vector<std::shared_ptr<Module>> mods)
		{
			auto ip = std::dynamic_pointer_cast<IPClientWin>(mods[0]);
			if (packet.status == 0)
			{
				ip->p0Handler->enqueue(UDP, packet, { boost::uuids::nil_uuid() });
			}
			else
			{
				ClientConnectedEvent ev;
				ev.address = ip->address;
				ip->connectEvent->fire(ev);
			}
		}, { shared_from_this() });
		connectEvent = viper->getModule<Events>("events")->initModule<EventHandler<ClientConnectedEvent>>("client_clientconnectedevent");
		disconnectEvent = viper->getModule<Events>("events")->initModule<EventHandler<ClientDisconnectedEvent>>("client_clientdisconnectedevent");
	};

	void onStartAsync() override
	{
		sleep = false;

		WSAData wsData;
		if (WSAStartup(0x202, &wsData))
		{
			crit("Client: WinSock could not be started: %d", WSAGetLastError());
			return;
		}

		udp = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (udp == INVALID_SOCKET)
		{
			crit("Client: UDP socket could not be created: %d", WSAGetLastError());
			return;
		}
		int timeout = 1;
		setsockopt(udp, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

		tcp = socket(AF_INET, SOCK_STREAM, 0);
		if (tcp == INVALID_SOCKET)
		{
			crit("Client: TCP socket could not be created: %d", WSAGetLastError());
			return;
		}

		udpAddr.sin_family = AF_INET;
		udpAddr.sin_port = htons(address.udpPort);
		inet_pton(AF_INET, address.address.c_str(), &udpAddr.sin_addr);

		tcpAddr = udpAddr;
		tcpAddr.sin_port = htons(address.tcpPort);

		FD_ZERO(&set);
		FD_SET(tcp, &set);

		info("Client: initialized");
	};

	void onTickAsync() override
	{
		if (!viper->isInitialized.load() || !viper->running.load())
		{
			return;
		}

		if (!connected.load())
		{
			info("Connecting to %s:%d...", address.address.c_str(), address.tcpPort);

			if (tcp == 0)
			{
				debug("Remaking socket...");
				tcp = socket(AF_INET, SOCK_STREAM, 0);
				if (tcp == INVALID_SOCKET)
				{
					crit("Client: TCP socket could not be created: %d", WSAGetLastError());
					return;
				}
			}

			if (connect(tcp, (sockaddr*)& tcpAddr, sizeof(tcpAddr)) == SOCKET_ERROR)
			{
				warn("Client: can't connect to server: %d - retrying in 1 second...", WSAGetLastError());
				std::this_thread::sleep_for(std::chrono::seconds(1));
				return;
			}
			set_atom(connected, true, bool);
		}

		p0Listener->poll();

		PacketWrapper<std::string> wrapper;
		while (outgoing.try_dequeue(wrapper))
		{
			if (wrapper.type == UDP)
			{
				auto data = std::to_string(wrapper.id) + wrapper.packet;
				int ok = sendto(udp, data.c_str(), (uint32)data.size() + 1, 0, (sockaddr*)& udpAddr, (int)sizeof(udpAddr));
				//debug("Client send UDP: %d bytes, WSA: %d", ok, WSAGetLastError());
			}
			else
			{
				auto data = std::to_string(wrapper.id) + wrapper.packet;
				int ok = send(tcp, data.c_str(), (uint32)data.size() + 1, 0);
				//debug("Client send TCP (%d): %d bytes, WSA: %d", (uint32)tcp, ok, WSAGetLastError());
			}
		}

		ZeroMemory(&buffer, 4096);

		sockaddr_in clientAddr;
		int clientSize = (int)sizeof(clientAddr);
		ZeroMemory(&clientAddr, clientSize);

		if (recvfrom(udp, buffer, 4096, 0, (sockaddr*)&clientAddr, &clientSize) > 0)
		{
			//debug("Client recv UDP: %s", buffer);

			auto packet = extract(buffer);
			if (packet.packet.empty())
			{
				warn("Client: invalid packet structure: %s", buffer);
				return;
			}

			incoming.enqueue(packet);
		}

		ZeroMemory(&buffer, 4096);
		TIMEVAL timeout = { 0, 1000 };
		int sockets = select(0, &set, nullptr, nullptr, &timeout);
		for (int i = 0; i < sockets; i++)
		{
			//debug("client select");
			int in = recv(tcp, buffer, 4096, 0);
			if (in > 0)
			{
				//debug("Client recv TCP: %s", buffer);

				auto packet = extract(buffer);
				if (packet.packet.empty())
				{
					warn("Client: invalid packet structure: %s", buffer);
					return;
				}
				incoming.enqueue(packet);
			}
			else
			{
				set_atom(connected, false, bool);
				ClientDisconnectedEvent ev;
				ev.reason = "CONNECTION_CLOSED";
				disconnectEvent->fire(ev);
				closesocket(tcp);
				tcp = 0;
				//debug("socket closed");
				return;
			}
		}

		FD_SET(tcp, &set); // THIS IS DUMB
		//debug("Client WSA: %d, sockets: %d", WSAGetLastError(), sockets);
	};

	void onStopAsync() override
	{
		closesocket(udp);
		closesocket(tcp);
		WSACleanup();
	};
};
