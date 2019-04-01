#pragma once
#include "Entity.h"
#include <boost/container/flat_map.hpp>

class World;
class V3;

namespace ECS
{
	class Container;

	class System : public ObjectBase
	{
	public:
		V3* v3;

		inline void preinit(V3* v3)
		{
			this->v3 = v3;
		};

		virtual void init(ECS::Container* container, World* world) { };

		void(*tickFunc)(double, Component*, System*, Container*, World*);
		void(*tickWait)(System*, World*);

		inline void setTickFunction(void(*tickFunc)(double, Component*, System*, Container*, World*))
		{
			this->tickFunc = tickFunc;
		};

		inline void setWaitFunction(void(*tickWait)(System*, World*))
		{
			this->tickWait = tickWait;
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