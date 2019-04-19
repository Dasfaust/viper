#pragma once
#include "../Module.h"
#include "TCPServer.h"
#include "boost/uuid/uuid.hpp"
#include <boost/uuid/uuid_generators.hpp>
#include "boost/container/flat_map.hpp"
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>
#include <atomic>

struct NetworkPlayer
{
	int socket;
	std::string nickname;
	boost::uuids::uuid uid;
};

class Networking : public Module
{
public:
	std::atomic<unsigned int> clients = 0;
	boost::container::flat_map<boost::uuids::uuid, NetworkPlayer> players;
	boost::container::flat_map<unsigned int, void(*)(Networking*, NetworkPlayer&, nlohmann::json data)> callbacks;

	Networking()
	{
		callbacks[1] = [](Networking* net, NetworkPlayer& player, nlohmann::json data)
		{
			player.nickname = data["nickname"].get<std::string>();

			nlohmann::json js;
			js["socket"] = data["socket"].get<int>();
			js["call"] = 1;
			js["nickname"] = player.nickname;

			net->send(player, js);
		};
	};

	void onStartup() override
	{
		server = std::make_shared<TCPServer>();
		server->start();
	};

	void onTick() override
	{
		int c;
		while(server->socketConnected.try_dequeue(c))
		{
			auto uid = boost::uuids::random_generator()();
			players[uid] = { c, "unknown", uid };
			debugf("NetworkPlayer created: %s", boost::lexical_cast<std::string>(uid).c_str());
			clients = clients + 1;
		}
		while (server->socketDisconnected.try_dequeue(c))
		{
			NetworkPlayer player;
			for (auto&& kv : players)
			{
				if ((&kv)->second.socket == c)
				{
					player = (&kv)->second;
					break;
				}
			}
			if (player.nickname.length() > 0)
			{
				players.erase(player.uid);
				debugf("NetworkPlayer deleted: %s", boost::lexical_cast<std::string>(player.uid).c_str());
				clients = clients - 1;
			}
		}

		nlohmann::json js;
		while(server->incoming.try_dequeue(js))
		{
			if (callbacks.count(js["call"].get<int>()))
			{
				NetworkPlayer player;
				for (auto&& kv : players)
				{
					if ((&kv)->second.socket == js["socket"].get<int>())
					{
						player = (&kv)->second;
						break;
					}
				}
				callbacks[js["call"].get<int>()](this, player, js);
			}
			else
			{
				warnf("Packet asks for callback %d but it doesn't exist", js["call"].get<int>());
			}
		}
	};

	void send(NetworkPlayer& player, nlohmann::json js)
	{
		js["socket"] = player.socket;
		server->outgoing.enqueue(js);
	};

	void send(nlohmann::json js)
	{
		js["socket"] = -1;
		server->outgoing.enqueue(js);
	};

	void onShutdown() override
	{
		server->stop();
	};
private:
	std::shared_ptr<TCPServer> server;
};