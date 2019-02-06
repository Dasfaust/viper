#include "V3.h"
#include "extensions/ExtDeltaTime.h"
#include "pipeline/RenderCommand.h"
#include "pipeline/Pipeline.h"
#include "events/EventLayer.h"
#include "Threadable.h"
#include "util/Time.h"
#include <unordered_set>

class Simulation : public Threadable
{
public:
    std::shared_ptr<EventListener<Pipeline::Camera::OnCameraMoveEventData>> listener;

    unsigned int tpsTarget = 0;
    double lastPoll = 0.0;
    double pollTime = 0.0;
    double deltaTime = 0.0;
    double accumulator = 0.0;

    double tickCount = 0.0;
    double perfAccumulator = 0.0;
    std::atomic<double> ticksPerSecond = 0.0;

    unsigned int renderId;
    unsigned int objectId;

    glm::vec3 cubePositions[10] = {
        glm::vec3( 0.0f,  0.0f,  0.0f), 
        glm::vec3( 2.0f,  5.0f, -15.0f), 
        glm::vec3(-1.5f, -2.2f, -2.5f),  
        glm::vec3(-3.8f, -2.0f, -12.3f),  
        glm::vec3( 2.4f, -0.4f, -3.5f),  
        glm::vec3(-1.7f,  3.0f, -7.5f),  
        glm::vec3( 1.3f, -2.0f, -2.5f),  
        glm::vec3( 1.5f,  2.0f, -2.5f), 
        glm::vec3( 1.5f,  0.2f, -1.5f), 
        glm::vec3(-1.3f,  1.0f, -1.5f)  
    };

    std::vector<int> objects;

    Simulation()
    {
        listener = std::make_shared<EventListener<Pipeline::Camera::OnCameraMoveEventData>>(
			[](Pipeline::Camera::OnCameraMoveEventData e) { /*debugf("OnCameraMoveEvent: %0.2f, %0.2f, %0.2f", e.cameraPos.x, e.cameraPos.y, e.cameraPos.z);*/ }
        );
        //V3::getInstance().getPipeline()->camera->moveEvent->addListener(listener);

        renderId = V3::getInstance()->getPipeline()->makeRenderCommand();
        /*auto command = V3::getInstance().getPipeline()->getRenderCommand(renderId);
        for (glm::vec3 location : cubePositions)
        {
            RenderCommand::WorldState state = { };
            state.worldCoordinates = location;
            state.rotation = -55.0f;
            state._rotation = -55.0f;
            state.scale = glm::vec3(1.0f, 0.0f, 0.0f);

            objectId = command->addObject(
                "cube",
                "",
                "container",
                0,
                "basic",
                { state }
            );
            objects.push_back(objectId);
        }*/
    };

    struct Chunk
    {
        int id = 0;
        int x = 0;
        int y = 0;
    };

    int worldHeight = 16;
    int worldWidth = 16;
    std::vector<Chunk> chunks;

    void onStart() override
    {
        #ifdef V3_WIN64
            tpsTarget = 30;
        #else
            // TODO: doesn't work in msvc. ???
            tpsTarget = V3::getInstance()->getConfig()->getInts("engine", "worldTickTarget")[0];
        #endif
        lastPoll = Time::now();
        deltaTime = 1000.0 / tpsTarget;
        debugf("Sim: TPS target: %d", tpsTarget);

        int worldSize = worldHeight * worldWidth;
        chunks.resize(worldSize);
        debugf("Sim: generating world: %d", worldSize);
        for (int x = 0; x < worldWidth; x++)
        {
            for (int y = 0; y < worldHeight; y++)
            {
                Chunk chunk = { };
                chunk.id = y * worldWidth + x;
                chunk.x = x;
                chunk.y = y;
                chunks[chunk.id] = chunk;

                RenderCommand::WorldState state = { };
                state.worldCoordinates = glm::vec3((float)chunk.x, 1.0f, (float)chunk.y);
                state._worldCoordinates = state.worldCoordinates;
                state.rotationX = 90.0f;
                state._rotationX = state.rotationX;
                state.scale = glm::vec3(1.0f, 1.0f, 1.0f);
                state._scale = state.scale;

                auto command = V3::getInstance()->getPipeline()->getRenderCommand(renderId);
                objectId = command->addObject(
                    "flat",
                    "",
                    "grass",
                    0,
                    "basic",
                    { state }
                );
                objects.push_back(objectId);
            }
        }

        debug("Sim: started.");
    };

    void tick() override
    {
        double newPoll = Time::now();
		pollTime = newPoll - lastPoll;
		lastPoll = newPoll;

        listener->poll();

        // Diagnostics
		if (perfAccumulator >= 1000.0)
		{
			ticksPerSecond = tickCount;
			//debugf("Sim: TPS = %.2f", ticksPerSecond.load());
			perfAccumulator = 0.0;
			tickCount = 0.0;
		}
		else
		{
			perfAccumulator += pollTime;
		}
		
		accumulator += pollTime;
		while(accumulator >= deltaTime)
		{
			for (int i = 1; i < 0; i++)
            {
                RenderCommand::WorldState state = V3::getInstance()->getPipeline()->getRenderCommand(renderId)->getObject(objects[i])->instances[0];
                state._worldCoordinates = state.worldCoordinates;
                // state.worldCoordinates.x += deltaTime * (0.0005f);

                state._rotationX = state.rotationX;
                state.rotationX += 0.00001f; /*V3::getInstance()->elapsedTime / 1000.0f * glm::radians(20.0f * i);*/
                debugf("%0.04f", state.rotationX);

                V3::getInstance()->getPipeline()->getRenderCommand(renderId)->getObject(objects[i])->instances[0] = state;
            }

            accumulator -= deltaTime;
            tickCount += 1;
		}

        double alpha =  accumulator / deltaTime;
        V3::getInstance()->getPipeline()->alpha.compare_exchange_strong(alpha, alpha);
        //debugf("alpha: %0.4f", V3::getInstance()->getPipeline()->alpha.load());
    };
};

int main()
{
    EXT_DELTA_TIME_LOAD();

	Simulation simulation;
    simulation.start();

    V3::getInstance()->start();

    simulation.stop();

    return 0;
}