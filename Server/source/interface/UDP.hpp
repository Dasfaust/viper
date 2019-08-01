#pragma once
#include "Defines.hpp"
#include "Viper.hpp"
#include "interface/Threadable.hpp"
#include "event/Events.hpp"
#include "PacketFactory.hpp"
#include "util/String.hpp"

struct Packet : Event
{
	uid client;
};

struct InetAddress
{
	std::string address;
	uint32 port;
};

class UDP : public Module, public Threadable
{
public:
	InetAddress address;
	moodycamel::ConcurrentQueue<PacketWrapper<std::string>> incoming;
	moodycamel::ConcurrentQueue<PacketWrapper<std::string>> outgoing;
	std::shared_ptr<Viper> viper;

	flatmap(std::string, uid) clientIds;
	flatmap(uid, InetAddress) clients;

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