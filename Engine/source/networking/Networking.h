#pragma once
#include "../Module.h"
#include "TCPServer.h"
#include "boost/uuid/uuid.hpp"
#include <boost/uuid/uuid_generators.hpp>
#include "boost/container/flat_map.hpp"
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>
#include <atomic>
#include "../util/Time.h"

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
	boost::container::flat_map<unsigned int, void(*)(Networking*, NetworkPlayer&, rapidjson::Document &data)> callbacks;
	std::atomic<double> worldUpdateMs;
	double lastWorldUpdate;

	Networking()
	{
		callbacks[1] = [](Networking* net, NetworkPlayer& player, rapidjson::Document &data)
		{
			/*player.nickname = std::string(data["nickname"].GetString());

			rapidjson::Document js;
			js.SetObject();
			js["socket"].SetInt(data["socket"].GetInt());
			js["call"].SetInt(1);
			js["nickname"].SetString(rapidjson::StringRef(player.nickname.c_str()));

			net->send(player, js);*/
		};
	};

	void onStartup() override
	{
		server = std::make_shared<TCPServer>();
		//server->start();
		server->onStart();
	};

	void _onTick()
	{
		int c;
		while (server->socketConnected.try_dequeue(c))
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

		Packet packet;
		while (server->incoming.try_dequeue(packet))
		{

			rapidjson::Document js;
			try
			{
				js.Parse(reinterpret_cast<const char*>(packet.message.c_str()));
			}
			catch (const std::exception& ex)
			{
				critf("JSON malformed");
				break;
			}

			if (callbacks.count(js["call"].GetInt()))
			{
				NetworkPlayer player;
				for (auto&& kv : players)
				{
					if ((&kv)->second.socket == packet.socket)
					{
						player = (&kv)->second;
						break;
					}
				}
				callbacks[js["call"].GetInt()](this, player, js);
			}
			else
			{
				warnf("Packet asks for callback %d but it doesn't exist", js["call"].GetInt());
			}
		}

		server->tick();
	}

	void onTick() override
	{
		
	};

	void send(NetworkPlayer& player, js::JsonObj& obj)
	{
		server->outgoing.enqueue({ player.socket, js::stringify(obj), obj.fields["call"].int_val });
	};

	void send(js::JsonObj& obj)
	{
		if (obj.fields["call"].int_val == 3)
		{
			worldUpdateMs = tnow() - lastWorldUpdate;
			//debugf("World update: %.2fms", worldUpdateMs.load());
			lastWorldUpdate = tnow();
			js::set(obj, "time", js::d(tnow()));
		}

		auto s = tnow();
		std::string js = js::stringify(obj);
		//debugf("Stringify took %.2f ms", tnow() - s);

		server->outgoing.enqueue({ -1, js, obj.fields["call"].int_val });
	};

	void onShutdown() override
	{
		server->onStop();
		//server->stop();
	};
private:
	std::shared_ptr<TCPServer> server;
};