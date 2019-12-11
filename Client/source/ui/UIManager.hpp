#pragma once
#include "interface/Module.hpp"
#include "../surface/WindowManager.hpp"

struct UICommandSet
{
	void(*func)(std::vector<std::shared_ptr<Module>>);
	std::vector<std::shared_ptr<Module>> modules;
};

class UIManager : public Module, public std::enable_shared_from_this<Module>
{
public:
	std::shared_ptr<WindowManager> wm;
	std::shared_ptr<Listener<ButtonPressedEvent>> buttonPressed;
	umap(std::string, UICommandSet) commands;

	void onStart() override;
	void onTickBegin() override;
	void onTickEnd() override;
	void onShutdown() override;

	void addCommand(std::string name, void(*func)(std::vector<std::shared_ptr<Module>>), std::vector<std::shared_ptr<Module>> modules = { })
	{
		commands[name] = { func, modules };
	};

	void removeCommand(std::string name)
	{
		commands.erase(name);
	};
};
