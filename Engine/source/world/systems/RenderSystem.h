#pragma once
#include "Components.h"
#include "../../ecs/ECS.h"
#include "../../pipeline/PipelineOpenGL.h"
#include "../../V3.h"
#include "../World.h"

class RenderSystem : public ECS::System
{
public:
	static Pipeline::WorldState worldState;
	Pipeline* pipeline;
	ECS::TypeInfo meshT, locT, moveT, camT;

	inline void init(ECS::Container* container, World* world) override
	{
		pipeline = world->v3->getModule<PipelineOpenGL>();
		meshT = container->resolveType<MeshComponent>();
		locT = container->resolveType<LocationComponent>();
		moveT = container->resolveType<MovementInputComponent>();
		camT = container->resolveType<CameraComponent>();

		setTickFunction([](double dt, ECS::Component* component, ECS::System* system, ECS::Container* container, World* world)
		{
			auto sys = reinterpret_cast<RenderSystem*>(system);
			ECS::Entity* entity = container->getEntity(component->entity);

			MeshComponent* mesh = nullptr;
			if (entity->components.count(sys->meshT.id))
			{
				mesh = container->getComponent<MeshComponent>(container->getEntity(component->entity));
				if (!RenderSystem::worldState.state.count(*mesh))
				{
					RenderSystem::worldState.state[{ mesh->type_id, mesh->type_size, mesh->index, mesh->entity, mesh->mesh, mesh->model, mesh->texture, mesh->shader, mesh->textureSlot, mesh->hash }] = boost::container::flat_map<uint32, Pipeline::Instance>();
				}
			}

			LocationComponent* loc = nullptr;
			if (entity->components.count(sys->locT.id))
			{
				loc = container->getComponent<LocationComponent>(container->getEntity(component->entity));
				if (mesh > 0)
				{
					RenderSystem::worldState.state[*mesh][entity->index] = { loc->type_id, loc->type_size, loc->index, loc->entity, loc->location };
				}
			}

			MovementInputComponent* move = nullptr;
			if (entity->components.count(sys->moveT.id))
			{
				move = container->getComponent<MovementInputComponent>(container->getEntity(component->entity));
				RenderSystem::worldState.player.movement = { move->type_id, move->type_size, move->index, move->entity, move->right, move->forward, move->pitch, move->yaw };
			}

			CameraComponent* cam = nullptr;
			if (entity->components.count(sys->camT.id))
			{
				cam = container->getComponent<CameraComponent>(container->getEntity(component->entity));
				cam->location = loc->location;
				cam->location.y += 3.0f;
				cam->location.z += -5.0f;
				RenderSystem::worldState.player.camera = { cam->type_id, cam->type_size, cam->index, cam->entity, cam->location };
			}
		});

		setWaitFunction([](ECS::System* system, World* world)
		{
			if (RenderSystem::worldState.initialized)
			{
				RenderSystem::worldState.stepAlpha = world->stepAlpha;
				world->v3->getModule<PipelineOpenGL>()->updateState(RenderSystem::worldState);
				RenderSystem::worldState = { false };
			}
		});
	};
private:
};

Pipeline::WorldState RenderSystem::worldState;