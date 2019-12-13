#pragma once
#include "Defines.hpp"
#include "event/Events.hpp"
#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>

#define make_serializable(...) template<class A> \
	void serialize(A& ar) \
	{ \
		ar(__VA_ARGS__); \
	} \

enum PACKET_TYPE
{
	UDP = 0,
	TCP = 1
};

template<typename T>
struct PacketWrapper
{
	uint32 id;
	PACKET_TYPE type;
	T packet;
	std::vector<uid> clients;
};

class PacketHandlerBase
{
public:
	uint32 id;
	PacketWrapper<std::string>(*pack)(std::shared_ptr<PacketHandlerBase>);
	void(*unpack)(std::shared_ptr<PacketHandlerBase>, std::string, uid);
};

template<typename T>
class PacketHandler : public PacketHandlerBase, public EventHandler<T>
{
public:
	moodycamel::ConcurrentQueue<PacketWrapper<T>> outgoing;

	void enqueue(PACKET_TYPE type, T& packet, std::vector<uid> clients = { })
	{
		PacketWrapper<T> wrap = { id, type, packet, clients };
		outgoing.enqueue(wrap);
	}
};

class PacketFactory : public std::enable_shared_from_this<PacketFactory>
{
public:
	umap(uint32, std::shared_ptr<PacketHandlerBase>) handlers;

	template<typename T>
	std::shared_ptr<PacketHandler<T>> registerPacket(uint32 id)
	{
		std::shared_ptr<PacketHandler<T>> handler = std::make_shared<PacketHandler<T>>();
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
				wrap.id = self->id;
				wrap.type = out.type;
				wrap.packet = ss.str() + "}"; // TODO: y tho?
				wrap.clients = out.clients;
			}
			return wrap;
		};

		handler->unpack = [](std::shared_ptr<PacketHandlerBase> self, std::string data, uid client)
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

	void packAll(moodycamel::ConcurrentQueue<PacketWrapper<std::string>>& queue)
	{
		for (auto&& kv : handlers)
		{
			bool empty = false;
			while (!empty)
			{
				auto wrapper = kv.second->pack(kv.second);
				if (!wrapper.packet.empty())
				{
					queue.enqueue(wrapper);
				}
				else
				{
					empty = true;
				}
			}
		}
	}

	void unpackAll(moodycamel::ConcurrentQueue<PacketWrapper<std::string>>& queue)
	{
		PacketWrapper<std::string> wrapper;
		while (queue.try_dequeue(wrapper))
		{
			if (handlers.count(wrapper.id))
			{
				handlers[wrapper.id]->unpack(handlers[wrapper.id], wrapper.packet, wrapper.clients.empty() ? "" : wrapper.clients[0]);
			}
		}
	}
};