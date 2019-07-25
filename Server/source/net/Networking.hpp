#pragma once
#include "interface/Threadable.hpp"
#include "interface/UDPServer.hpp"
#include "log/Logger.hpp"
#include "event/Events.hpp"
#include <cereal/archives/json.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

#ifdef VIPER_WIN64
#include "WinUDP.hpp"
#endif

class Networking;
typedef boost::uuids::uuid clientid;

struct NetworkClient
{
	int socket;
};

struct Packet : Event
{
	clientid client;
};

template<typename T>
struct PacketWrapper
{
	T packet;
	std::vector<clientid> clients;
};

class PacketHandlerBase
{
public:
	uint32 id;
	// grab from outgoing, serialize
	PacketWrapper<std::string>(*pack)(std::shared_ptr<PacketHandlerBase>);
	// deserialize, throw event
	void(*unpack)(std::shared_ptr<PacketHandlerBase>, std::string, clientid);
};

template<typename T>
class PacketHandler : public PacketHandlerBase, public EventHandler<T>
{
public:
	moodycamel::ConcurrentQueue<PacketWrapper<T>> outgoing;

	void enqueue(T& packet, std::vector<clientid> clients = { })
	{
		PacketWrapper<T> wrap = { packet, clients };
		outgoing.enqueue(wrap);
	}
};

class Networking : public Module, public Modular, public Threadable
{
public:
	flatmap(uint32, std::shared_ptr<PacketHandlerBase>) handlers;
	std::shared_ptr<UDPServer> udp;

	void onStart() override
	{
#ifdef VIPER_WIN64
		initModule<WinUDP>("udp");
#endif

		for (auto&& kv : modules)
		{
			kv.second->onStart();
		}

		udp = getModule<UDPServer>("udp");
	};

	void onTick() override
	{
		for (auto&& kv : handlers)
		{
			std::vector<PacketWrapper<std::string>> outgoing;
			bool empty = false;
			while(!empty)
			{
				auto wrapper = kv.second->pack(kv.second);
				if (!wrapper.packet.empty())
				{
					outgoing.push_back(wrapper);

					kv.second->unpack(kv.second, wrapper.packet, boost::uuids::random_generator()());
				}
				else
				{
					empty = true;
				}
			}
		}

		tickModules();
	};

	template<typename T>
	std::shared_ptr<PacketHandler<T>> registerPacket(uint32 id)
	{
		std::shared_ptr<PacketHandler<T>> handler = std::make_shared<PacketHandler<T>>();
		handler->parent = this;
		handler->id = id;

		handler->pack = [](std::shared_ptr<PacketHandlerBase> self) -> PacketWrapper<std::string>
		{
			PacketWrapper<std::string> wrap;
			std::shared_ptr<PacketHandler<T>> inst = std::static_pointer_cast<PacketHandler<T>>(self);
			PacketWrapper<T> out;
			if (inst->outgoing.try_dequeue(out))
			{
				std::stringstream ss;
				cereal::JSONOutputArchive archive(ss);
				out.packet.serialize(archive);
				wrap.packet = ss.str() + "}"; // TODO: y tho?
				wrap.clients = out.clients;
			}
			return wrap;
		};

		handler->unpack = [](std::shared_ptr<PacketHandlerBase> self, std::string data, clientid client)
		{
			std::shared_ptr<PacketHandler<T>> inst = std::static_pointer_cast<PacketHandler<T>>(self);
			std::stringstream ss(data);
			cereal::JSONInputArchive archive(ss);
			T packet;
			packet.serialize(archive);
			packet.client = client;
			inst->fire(packet);
		};

		handlers[id] = handler;
		return handler;
	};
};
