#include "Viper.hpp"
#include "event/Events.hpp"
#include "log/Logger.hpp"
#include "log/Logging.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "ecs/ECS.hpp"
#include "glm/vec3.hpp"

struct Comp1
{
	float data = 3.14f;

	make_serializable(CEREAL_NVP(data));
};

struct Comp2
{
	std::string data = "dickbutt";

	make_serializable(CEREAL_NVP(data));
};

class Sandbox : public Module, public Modular
{
public:
	Sandbox()
	{
		friendlyName = "Sandbox";
	};

	void onStart() override
	{
		for (auto&& kv : modules)
		{
			kv.second->onStart();
		}
	};

	void onTick() override
	{
		
	};

	void onShutdown() override
	{
		for (auto&& kv : modules)
		{
			kv.second->onShutdown();
		}
	};
};

int main()
{
	Logging::async = true;
	auto vi = std::make_shared<Viper>();
	auto lo = vi->initModule<Logging>("logging");
	auto th = vi->initModule<Threads>("threads");
	auto ev = vi->initModule<Events>("events");
	auto sr = vi->initModule<Server>("server");
	auto cl = vi->initModule<Client>("client");
	auto ga = vi->initModule<Sandbox>("game");
	vi->start();
};