#include "Viper.hpp"
#include "event/Events.hpp"
#include "log/Logger.hpp"
#include "log/Logging.hpp"
#include "Server.hpp"
#include "Client.hpp"

class Sandbox : public Module
{
public:
	Sandbox()
	{
		friendlyName = "Sandbox";
	};

	void onStart() override
	{
		
	};

	void onTick() override
	{
		
	};
};

int main()
{
	auto vi = std::make_shared<Viper>();
	auto th = vi->initModule<Threads>("threads");
	auto lo = vi->initModule<Logging>("logging");
	auto ev = vi->initModule<Events>("events");
	auto sr = vi->initModule<Server>("server");
	auto cl = vi->initModule<Client>("client");
	auto ga = vi->initModule<Sandbox>("game");
	vi->start();
};