#pragma once
#include "../Module.h"
#include "TCPServer.h"

class Networking : public Module
{
public:
	void onStartup() override
	{
		server = std::make_shared<TCPServer>();
		server->start();
	};

	void onShutdown() override
	{
		server->stop();
	};
private:
	std::shared_ptr<TCPServer> server;
};