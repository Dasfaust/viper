#pragma once
#include <boost/container/flat_map.hpp>
#include "../util/Any.h"
#include <typeindex>
#include "../memory/Pool.h"

class World;
class V3;

namespace ECS
{
	class Container;

	struct Changeset : public object
	{
		uint32 index;
		uint32 field = 0;
		uint32 worker;
	};

	class System : public ObjectBase
	{
	public:
		V3* v3;

		inline void preinit(V3* v3)
		{
			this->v3 = v3;

			tickFunc = [](double dt, Component* comp, System* sys, Container* cont, World* world, unsigned int worker) { };
			tickEnd = [](System* sys, World* world) { };
			tickWait = [](System* sys, World* world) { };
		};

		virtual void init(ECS::Container* container, World* world) { };

		void(*tickFunc)(double, Component*, System*, Container*, World*, unsigned int);
		void(*tickWait)(System*, World*);
		void(*tickEnd)(System*, World*);
		void(*applyChange)(std::string, Component*, Changeset*, World* world, System* system);

		inline void setTickFunction(void(*tickFunc)(double, Component*, System*, Container*, World*, unsigned int))
		{
			this->tickFunc = tickFunc;
		};

		inline void setWaitFunction(void(*tickWait)(System*, World*))
		{
			this->tickWait = tickWait;
		};

		inline void setTickEndFunction(void(*tickEnd)(System*, World*))
		{
			this->tickEnd = tickEnd;
		};

		inline void setApplyChangeFunction(void(*applyChange)(std::string, Component*, Changeset*, World* world, System* system))
		{
			this->applyChange = applyChange;
		};

		inline void addType(uint8 id, size_t size)
		{
			types[id] = size;
		};

		inline boost::container::flat_map<uint8, size_t>& getTypes()
		{
			return types;
		};
	private:
		boost::container::flat_map<uint8, size_t> types;
	};
};