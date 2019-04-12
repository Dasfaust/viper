#pragma once
#include "../Module.h"
#include "../Threadable.h"
#include "../Logger.h"
#include "../util/String.h"
#include "boost/container/flat_map.hpp"

class ConsoleInput;

class CommandHandler
{
public:
	virtual std::string getHelpText() { return ""; };
	virtual unsigned int getMinLength() { return 0; };
	virtual void execute(ConsoleInput* input, std::vector<std::string>& command) { };
};

class ConsoleInput : public Module, Threadable
{
public:
	class StopCommand : public CommandHandler
	{
	public:
		std::string getHelpText() override
		{
			return "stop [stops the world server]";
		}

		inline void execute(ConsoleInput* input, std::vector<std::string>& command) override
		{
			input->v3->shutdown();
		};
	};

	inline void onStartup() override
	{
		registerCommand("stop", std::make_shared<StopCommand>());
		this->start();
	};

	inline void tick() override
	{
		if (!shutdown)
		{
			std::string command;
			std::getline(std::cin, command);
			auto cmd = splitString(command, ' ');
			if (!cmd.empty())
			{
				if (handlers.count(cmd[0]))
				{
					auto handler = handlers[cmd[0]];
					handler->execute(v3->getModule<ConsoleInput>(), cmd);

					if (cmd[0] == "stop")
					{
						shutdown = true;
					}
				}
				else if (cmd[0] == "help" || cmd[0] == "?")
				{
					info("Command help:");
					for (auto kv : handlers)
					{
						info((&kv)->second->getHelpText());
					}
				}
				else
				{
					infof("Uknown command: %s", command.c_str());
				}
			}
		}
	};

	inline void onShutdown() override
	{
		this->stop();
	};

	inline void registerCommand(std::string prefix, std::shared_ptr<CommandHandler> handler)
	{
		handlers[prefix] = handler;
	};
private:
	bool shutdown = false;
	boost::container::flat_map<std::string, std::shared_ptr<CommandHandler>> handlers;
};