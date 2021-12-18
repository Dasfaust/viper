# Viper
Viper is a WIP learning project. It aims to be a light weight, multi-player first 2D & 3D game engine.

## The Master Plan
Viper's archetecture is broken into a few simple concepts. There's a server, a client, and multiple game worlds.

#### Server
The server's job is to process all gameplay actions and world simulation at a set framerate and then send a state update to the connected clients.

#### Worlds
Contained by the server, handles every object in the world. Much like a scene in other game engines.

#### Client
The client's job is to take the state information and present it, interpolating between each server step so that the framerate is decoupled from the server.

## Implementation
Viper is more like a framework, a collection of modules that each perform their own tasks. Modules can, however, depend on one another. Initializing the application looks like this:
```cpp
auto vi = std::make_shared<Viper>();
// Provides a thread-safe logger
vi->initModule<Logging>("logging");
// Thread management
vi->initModule<Threads>("threads");
// A thread-safe event loop
vi->initModule<Events>("events");
// The server module
vi->initModule<Server>("server");
// The client module
vi->initModule<Client>("client");
// And finally, the game module itself
vi->initModule<Sandbox>("game");
vi->start();
```

#### Thread Safety
Thread safety is achieved by using moodycamel's wonderful lock-free queue in a push and pull arrangement. The logger, for instance, takes in messages through a queue and reads (and outputs) the items on its respective thread.

Events are handled much the same way:
```cpp
// Define event parameters
struct SomeEventData
{
	int someInt;
}

// Register the event and get the handler
auto someEvent = events->initModule<EventHandler<SomeEvent>>("someEvent");

// Create a listener for the event along with its position in the list and a callback function
// Optionally pass in any module
auto someEventListener = someEvent->listen(0, [](SomeEventData& data, std::vector<std::shared_ptr<Module>> modules)
{
	auto module = std::dynamic_pointer_cast<SomeModule>(modules[0]);
	module->someData += data->someInt;
}, { getModule("SomeModule") });

...

// Fire the event
SomeEventData data { 42 };
someEvent->fire(data);

...

// Poll for the result
// Once the event is fired, the callback will be executed on whichever thread poll() is called from
someEventListener->poll();
```

Networking is handled via the event system. Socket listeners, and servers, run on their own threads and fire an event when a packet is recieved. The main client/server thread then processes that event, and in turn, the packet data.

#### ECS
The game world (and the corresponding client scene) is built upon an entity component system. The methodology behind this is that entities are nothing more than an id, components are just data, and the systems handle all of the logic.

In memory, each entity looks something like this:
```
[id][component 1][component 2][id][component 1][component 2]...
```
This allows each entity to be updated extremely quickly, minimizing cache misses.
```cpp
// To iterate entities, just increment the index by the entity size
// Entity size is the size of the identifier + each possible component size
// heap, in this case is the vector containing the raw data
// This may be wasteful memory-wise, but it doesn't seem to be an issue at the moment
for (uint64 i = 0; i < heap.size(); i += entitySize)
{
	auto ent = reinterpret_cast<Entity*>(&heap[i]);
	...
}
```
