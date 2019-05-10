#pragma once
#include <boost/container/flat_map.hpp>
#include "../util/Any.h"

class World;
class V3;

namespace ECS
{
	class Container;

	struct Changeset
	{
		uint32 index;
		uint32 field;
		Any value;
	};

	class System : public ObjectBase
	{
	public:
		V3* v3;

		inline void preinit(V3* v3)
		{
			this->v3 = v3;

			tickFunc = [](double dt, Component* comp, System* sys, Container* cont, World* world) {};
			tickEnd = [](System* sys, World* world) { };
			tickWait = [](System* sys, World* world) { };
		};

		virtual void init(ECS::Container* container, World* world) { };

		void(*tickFunc)(double, Component*, System*, Container*, World*);
		void(*tickWait)(System*, World*);
		void(*tickEnd)(System*, World*);
		void(*applyChange)(std::string, Component*, Changeset, World* world, System* system);

		inline void setTickFunction(void(*tickFunc)(double, Component*, System*, Container*, World*))
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

		inline void setApplyChangeFunction(void(*applyChange)(std::string, Component*, Changeset, World* world, System* system))
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