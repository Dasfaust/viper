#pragma once
#include "Defines.hpp"
#include "Viper.hpp"
#include "interface/Threadable.hpp"
#include "event/Events.hpp"
#include "PacketFactory.hpp"
#include "util/String.hpp"
#include "../net/Packets.hpp"

struct InetAddress
{
	std::string address;
	uint32 udpPort;
	uint32 tcpPort;
	uint32 socket;
};

struct ClientConnectedEvent : Event
{
	uid id;
	InetAddress address;
};

struct ClientDisconnectedEvent : Event
{
	uid id;
	std::string reason;
};

class IPHandler : public Module, public Threadable
{
public:
	std::atomic_int64_t deltaTime;
	std::atomic_int64_t tickTime;
	std::atomic_int64_t incomingTime;
	std::atomic_int64_t outgoingTime;
	
	InetAddress address;
	std::shared_ptr<PacketFactory> factory;
	moodycamel::ConcurrentQueue<PacketWrapper<std::string>> incoming;
	moodycamel::ConcurrentQueue<PacketWrapper<std::string>> outgoing;
	std::shared_ptr<Viper> viper;

	umap(uint32, uid) tokens;
	umap(uid, InetAddress) clients;
	umap(std::string, uid) clientHosts;

	std::shared_ptr<PacketHandler<P0Handshake>> p0Handler;
	std::shared_ptr<EventHandler<ClientConnectedEvent>> connectEvent;
	std::shared_ptr<EventHandler<ClientDisconnectedEvent>> disconnectEvent;

	PacketWrapper<std::string> extract(char buffer[4096])
	{
		PacketWrapper<std::string> packet;

		std::string rec = std::string(buffer);
		std::string pid;
		for (uint32 i = 0; i < rec.size(); i++)
		{
			if (rec[i] == '{')
			{
				break;
			}

			pid += rec[i];
		}

		if (!isNumber(pid))
		{
			return packet;
		}

		uint32 id = std::stoi(pid);
		packet.id = id;
		packet.packet = rec.substr(pid.length(), rec.size() - 1);

		return packet;
	}
};