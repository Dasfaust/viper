#include "V3.h"
#include "extensions/ExtDeltaTime.h"
#include "pipeline/RenderCommand.h"
#include "pipeline/Pipeline.h"
#include "events/EventLayer.h"
#include "Threadable.h"
#include "util/Time.h"
#include <unordered_set>
#include "imgui.h"
#include "pipeline/imgui_impl_opengl3.h"

class ImGuiRenderer : public RenderCommand
{
public:
	bool show = true;
    std::atomic<bool> simTps = 0.0;

	ImGuiRenderer()
	{
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGuiIO& io = ImGui::GetIO();
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

		io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
		io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
		io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
		io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
		io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
		io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
		io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
		io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
		io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
		io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
		io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
		io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
		io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
		io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
		io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
		io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
		io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
		io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
		io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
		io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
		io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

		ImGui_ImplOpenGL3_Init("#version 410");
	}

	void tick() override
	{
		ImGuiIO& io = ImGui::GetIO();
		// Takes seconds. Debugged this for hours
		io.DeltaTime = V3::getInstance()->getPipeline()->deltaTime / 1000.0f;
		io.DisplaySize = ImVec2(V3::getInstance()->getView()->viewWidth, V3::getInstance()->getView()->viewHeight);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();
		
		//ImGui::ShowDemoWindow(&show);

        ImGui::SetNextWindowSize(ImVec2(350, 250));
        if (!ImGui::Begin("World State", &show))
        {
            // Early out if the window is collapsed, as an optimization.
            ImGui::End();
            return;
        }
        auto pipeline = V3::getInstance()->getPipeline();
        auto camera = pipeline->camera;
        ImGui::Text("Frame time: %d, %.2fms", V3::getInstance()->framesPerSecond, V3::getInstance()->deltaTime);
        ImGui::Text("Camera floor: %.2f, %.2f, %.2f", camera->floorPosition.x, camera->floorPosition.y, camera->floorPosition.z);
        ImGui::Text("Camera pos: %.2f, %.2f, %.2f", camera->position.x, camera->position.y, camera->position.z);
        ImGui::Text("Camera front: %.2f, %.2f, %.2f", camera->front.x, camera->front.y, camera->front.z);
        ImGui::Text("Cursor to world: %.2f, %.2f, %.2f", camera->mouseWorld.x, camera->mouseWorld.y, camera->mouseWorld.z);
        RenderCommand::WorldState state = V3::getInstance()->getPipeline()->getRenderCommand(0)->getObject(1)->instances[0];
        ImGui::Text("Pos/prev pos: %.2f, %.2f, %.2f | %.2f, %.2f, %.2f", state.worldCoordinates.x, state.worldCoordinates.y, state.worldCoordinates.z, state._worldCoordinates.x, state._worldCoordinates.y, state._worldCoordinates.z);
        ImGui::End();

        /*ImGui::SetNextWindowSize(ImVec2(350, 250));
        if (!ImGui::Begin("Simulation State", &show))
        {
            // Early out if the window is collapsed, as an optimization.
            ImGui::End();
            return;
        }
        ImGui::Text("Poll time: %d, %.2fms", V3::getInstance()->framesPerSecond, V3::getInstance()->deltaTime);
        ImGui::End();*/


		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
};

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
    unsigned int terrainId;
    unsigned int objectId;
    std::vector<int> objects;

    glm::vec3 cubePositions[2] = {
        glm::vec3( 0.0f,  1.5f,  0.0f), 
        glm::vec3( 0.0f,  1.0f,  0.0f)
        /*glm::vec3(-1.5f, -2.2f, -2.5f),  
        glm::vec3(-3.8f, -2.0f, -12.3f),  
        glm::vec3( 2.4f, -0.4f, -3.5f),  
        glm::vec3(-1.7f,  3.0f, -7.5f),  
        glm::vec3( 1.3f, -2.0f, -2.5f),  
        glm::vec3( 1.5f,  2.0f, -2.5f), 
        glm::vec3( 1.5f,  0.2f, -1.5f), 
        glm::vec3(-1.3f,  1.0f, -1.5f)*/
    };

    Simulation()
    {
        listener = std::make_shared<EventListener<Pipeline::Camera::OnCameraMoveEventData>>(
			[](Pipeline::Camera::OnCameraMoveEventData e) { /*debugf("OnCameraMoveEvent: %0.2f, %0.2f, %0.2f", e.cameraPos.x, e.cameraPos.y, e.cameraPos.z);*/ }
        );
        //V3::getInstance().getPipeline()->camera->moveEvent->addListener(listener);

        renderId = V3::getInstance()->getPipeline()->makeRenderCommand();
        debugf(
            "renderer: %d",
            renderId
        );
        auto command = V3::getInstance()->getPipeline()->getRenderCommand(renderId);
        for (glm::vec3 location : cubePositions)
        {
            RenderCommand::WorldState state = { };
            state.worldCoordinates = location;
            state._worldCoordinates = state.worldCoordinates;
            state.rotationX = 90.0f;
            state._rotationX = state.rotationX;
            state.scale = glm::vec3(1.0f, 1.0f, 1.0f);
            state._scale = state.scale;

            objectId = command->addObject(
                "cube",
                "",
                "container",
                0,
                "basic",
                { state }
            );
            objects.push_back(objectId);
            debugf("%d", objectId);
        }

        auto imgui = std::make_shared<ImGuiRenderer>();
        V3::getInstance()->getPipeline()->addRenderCommand(imgui);
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
            //tpsTarget = V3::getInstance()->getConfig()->getInts("engine", "worldTickTarget")[0];
        #endif

        tpsTarget = 36;
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
            RenderCommand::WorldState state = V3::getInstance()->getPipeline()->getRenderCommand(renderId)->getObject(1)->instances[0];
            state._worldCoordinates = state.worldCoordinates;
            state.worldCoordinates = V3::getInstance()->getPipeline()->camera->floorPositionTarget;
            state._rotationX = state.rotationX;
            state.rotationX += 5.0f;
            state._rotationZ = state.rotationX;
            state.rotationZ += 5.0f;

            V3::getInstance()->getPipeline()->getRenderCommand(renderId)->getObject(1)->instances[0] = state;

            state = V3::getInstance()->getPipeline()->getRenderCommand(renderId)->getObject(2)->instances[0];
            state._rotationX = state.rotationX;
            state.rotationX += 3.5f;
            state._rotationZ = state.rotationX;
            state.rotationZ += 3.5f;

            V3::getInstance()->getPipeline()->getRenderCommand(renderId)->getObject(2)->instances[0] = state;

            //debugf("Sim: mouseToWorld: %0.2f, %0.2f, %0.2f", V3::getInstance()->getPipeline()->camera->mouseWorld.x, V3::getInstance()->getPipeline()->camera->mouseWorld.y, V3::getInstance()->getPipeline()->camera->mouseWorld.z);
			for (int i = 1; i < 0; i++)
            {
                RenderCommand::WorldState state = V3::getInstance()->getPipeline()->getRenderCommand(renderId)->getObject(objects[i])->instances[0];
                //state._worldCoordinates = state.worldCoordinates;
                // state.worldCoordinates.x += deltaTime * (0.0005f);

                state._rotationX = state.rotationX;
                state.rotationX += 0.1f; /*V3::getInstance()->elapsedTime / 1000.0f * glm::radians(20.0f * i);*/
                //debugf("%0.04f", state.rotationX);

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