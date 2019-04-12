#pragma once
#include "ECS.h"
#include "../console/ConsoleInput.h"
#include "../Logger.h"
#include "../V3.h"
#include "../world/World.h"

class ECSCommand : public CommandHandler
{
public:
	std::string getHelpText() override
	{
		return "ecs [see ecs for usage]";
	};

	inline void execute(ConsoleInput* input, std::vector<std::string>& command) override
	{
		if (command.size() == 1)
		{
			info("ECS command help: ");
			info(" -> types [list all ECS types]");
			return;
		}

		if (command[1] == "types")
		{
			info("Registered ECS types: ");
			for (auto kv : input->v3->getModule<World>()->ecs->getTypes())
			{
				infof(" -> %d as %s", (&kv)->second, (&kv)->first.c_str());
			}
			return;
		}
	};
};