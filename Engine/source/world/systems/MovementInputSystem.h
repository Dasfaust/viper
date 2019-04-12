#pragma once
#include "Components.h"
#include "../World.h"
#include "../../view/ViewLayer.h"
#include "../../V3.h"

class MovementInputSystem : public ECS::System
{
public:
	std::shared_ptr<EventListener<ViewEvents::KeyEvent>> keyEvent;

	static float lastForward;
	static float lastRight;

	inline void init(ECS::Container* container, World* world) override
	{
		//debug("Movement init");
		setTickFunction([](double dt, ECS::Component* component, ECS::System* system, ECS::Container* container, World* world)
		{
			//auto comp = reinterpret_cast<MovementInputComponent*>(component);
			//comp->forward = lastForward;
			//comp->right = lastRight;
			//debugf("MoveInputComponent: forward %0.2f right %.02f", comp->forward, comp->right);
		});

		setWaitFunction([](ECS::System* system, World* world)
		{
			//auto inst = reinterpret_cast<MovementInputSystem*>(system);
			//inst->keyEvent->poll(system->v3->getModule<ViewLayer>()->keyEvent);
		});

		/*keyEvent = v3->getModule<ViewLayer>()->keyEvent->listen([](ViewEvents::KeyEvent* data)
		{
			// TODO acceleration?

			if (data->buttons[GLFW_KEY_W].pressed)
			{
				lastForward = 1.0f;
			}
			else if (data->buttons[GLFW_KEY_S].pressed)
			{
				lastForward = -1.0f;
			}
			else
			{
				lastForward = 0.0f;
			}

			if (data->buttons[GLFW_KEY_A].pressed)
			{
				lastRight = -1.0f;
			}
			else if (data->buttons[GLFW_KEY_D].pressed)
			{
				lastRight = 1.0f;
			}
			else
			{
				lastRight = 0.0f;
			}
		});*/
	};
private:
};

float MovementInputSystem::lastForward = 0.0f;
float MovementInputSystem::lastRight = 0.0f;