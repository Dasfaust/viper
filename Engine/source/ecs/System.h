#pragma once
#include "Base.h"
#include <boost/container/flat_map.hpp>

namespace ECS
{
	class Container;

	class System : public ObjectBase
	{
	public:
		inline void init(void(*tickFunc)(double, Component*, System*, Container*), boost::container::flat_map<uint8, size_t>& types)
		{
			this->tickFunc = tickFunc;
			this->types = types;
		};

		inline void tick(double dt, Component* component, Container* container)
		{
			tickFunc(dt, component, this, container);
		};

		inline boost::container::flat_map<uint8, size_t>& getTypes()
		{
			return types;
		};
	private:
		boost::container::flat_map<uint8, size_t> types;
		void(*tickFunc)(double, Component*, System*, Container*);
	};
};