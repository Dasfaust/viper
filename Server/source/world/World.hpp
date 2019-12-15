#pragma once
#include "ecs/Container.hpp"
#include "interface/IPHandler.hpp"

struct Location
{
	vec3 position;
};

class World : public Module, public Modular
{
public:
	std::shared_ptr<ecs::Container> container;
	std::shared_ptr<Listener<ClientConnectedEvent>> clientConnected;
	bool firstTick = true;
	
	void onStart() override
	{
		container = initModule<ecs::Container>("container");
		container->registerComponent<Location>();

		container->onStart();
	};
	
	void onTick() override
	{
		if (firstTick)
		{
			auto ccHandler = getParent<Module>()->getParent<Modular>()->getModule<Events>("events")->getModule<EventHandler<ClientConnectedEvent>>("server_clientconnectedevent");
			clientConnected = ccHandler->listen(10, [](ClientConnectedEvent& ev, std::vector<std::shared_ptr<Module>> mods)
			{
				// todo: send population packet
			}, {});
			
			firstTick = false;
		}
		
		clientConnected->poll();
		tickModules();
	};
};