#include <Godot.hpp>
#include <MainLoop.hpp>
#include <Node.hpp>
#include <SceneTree.hpp>
#include "Viper.hpp"
#include "event/Events.hpp"
#include "log/Logger.hpp"
#include "log/Logging.hpp"
#include "Server.hpp"
#include "Client.hpp"

using namespace godot;

inline void pollViperLogger()
{
	viper::pollLogger([](std::string msg)
	{
		Godot::print(msg.c_str());
	});
};

class Sandbox : public Module
{
public:
};

class ViperNode : public Node
{
	GODOT_CLASS(ViperNode, Node);
public:
	std::shared_ptr<Viper> vi;

	void _init()
	{
		vi = std::make_shared<Viper>();
		auto th = vi->initModule<Threads>("threads");
		auto ev = vi->initModule<Events>("events");
		auto sr = vi->initModule<Server>("server");
		auto cl = vi->initModule<Client>("client");
		auto ga = vi->initModule<Sandbox>("game");
		vi->onStart();
	};

	void _ready()
	{
		//get_tree()->set_auto_accept_quit(false);
	};

	void _process(float delta)
	{
		pollViperLogger();
		vi->tickModules();
	};

	void _notification(int what)
	{
		// TODO: not sure if this works
		if (what == MainLoop::NOTIFICATION_WM_QUIT_REQUEST || what == MainLoop::NOTIFICATION_WM_GO_BACK_REQUEST)
		{
			vi->onShutdown();
			pollViperLogger();
			get_tree()->quit();
		}
	};

	static void _register_methods()
	{
		register_method("_ready", &ViperNode::_ready);
		register_method("_notification", &ViperNode::_notification);
		register_method("_process", &ViperNode::_process);
	};
};

extern "C" void GDN_EXPORT godot_gdnative_init(godot_gdnative_init_options* o)
{
	godot::Godot::gdnative_init(o);
}

extern "C" void GDN_EXPORT godot_gdnative_terminate(godot_gdnative_terminate_options* o)
{
	godot::Godot::gdnative_terminate(o);
}

extern "C" void GDN_EXPORT godot_nativescript_init(void* handle)
{
	godot::Godot::nativescript_init(handle);

	godot::register_class<ViperNode>();
}