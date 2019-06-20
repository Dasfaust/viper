#pragma once
#include "interface/Threadable.hpp"
#include "interface/TCPServer.hpp"
#include "util/Memory.hpp"
#include "log/Logger.hpp"

#ifdef VIPER_WIN64
#include "WinTCP.hpp"
#endif

#define pktfield(a) hPacket->a = sPacket.a;

#define mkpktbuilder(a, b, c, d) a = std::make_shared<PacketBuilder<TestPacket>>(); \
		a->setBuildFunction([](std::shared_ptr<Networking> net, std::shared_ptr<BuilderBase> build) -> object* { \
		auto& queue = std::reinterpret_pointer_cast<PacketBuilder<b>>(build)->queue; \
		b sPacket; \
		b* hPacket = nullptr; \
		if (queue.try_dequeue(sPacket)) { \
		hPacket = net->pool.create<b>(); \
		hPacket->name = std::string(sPacket.name); \
		d } \
		return hPacket; \
		}); \
		c->getModule<Server>("server")->getModule<Networking>("networking")->registerBuilder<PacketBuilder<TestPacket>>(builder)

class Networking;

struct Packet : object
{
	std::string name;
};

class BuilderBase
{
public:
	object*(*build)(std::shared_ptr<Networking>, std::shared_ptr<BuilderBase>);

	void setBuildFunction(object*(*build)(std::shared_ptr<Networking>, std::shared_ptr<BuilderBase>))
	{
		this->build = build;
	};
};

template<typename T>
class PacketBuilder : public BuilderBase
{
public:
	moodycamel::ConcurrentQueue<T> queue;

	void enqueue(T& data)
	{
		queue.enqueue(data);
	};
};

class Networking : public Module, public Modular, public Threadable
{
public:
	StaticPool pool;
	flatmap(std::type_index, std::shared_ptr<BuilderBase>) builders;
	std::shared_ptr<TCPServer> server;

	void onStart() override
	{
#ifdef VIPER_WIN64
		initModule<WinTCP>("tcp");
#endif

		for (auto&& kv : modules)
		{
			kv.second->onStart();
		}

		server = getModule<TCPServer>("tcp");
	};

	void onShutdown() override
	{
		pool.purge();
	};

	void onTick() override
	{
		flatmap(uint32, std::vector<uint32>) created;

		for (auto&& kv : builders)
		{
			// INTERESTINGLY ... creating an object on the pool invalidates previous pointers returned by create()
			object* ob = kv.second->build(getParent<Modular>()->getModule<Networking>("networking"), kv.second);
			while (ob != 0)
			{
				created[ob->type->id].push_back(ob->id);

				debug("Packet ID: %d, type: %d, size: %d, name: %s", ob->id, ob->type->id, ob->type->size, reinterpret_cast<Packet*>(ob)->name.c_str());

				std::vector<uint32> raw;
				std::string sraw;
				for (uint32 i = ob->id; i < ob->id + (uint32)ob->type->size; i++)
				{
					raw.push_back(pool.heaps[ob->type->id][i]);
					sraw += std::to_string(i);
				}
				debug("Raw: %s", sraw.c_str());

				ob = kv.second->build(getParent<Modular>()->getModule<Networking>("networking"), kv.second);
			}
		}

		tickModules();

		for (auto&& kv : created)
		{
			for (uint32 id : kv.second)
			{
				pool.del(pool.getType(kv.first), id);
			}

			debug("%d", pool.freeSlots[kv.first].size());
		}
	};

	template<typename T>
	void registerBuilder(std::shared_ptr<T> builder)
	{
		builders[std::type_index(typeid(T))] = builder;
	};

	template<typename T>
	std::shared_ptr<PacketBuilder<T>> getBuilder()
	{
		return std::reinterpret_pointer_cast<PacketBuilder<T>>(builders[std::type_index(typeid(PacketBuilder<T>))]);
	};
};
