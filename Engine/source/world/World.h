#include "../EngineExtension.h"
#include "../Threadable.h"
#include "System.h"
#include <unordered_map>

#define EXT_WORLD_ADD() V3::getInstance()->addExtension(World::getInstance(), 0.0);

class World : public EngineExtension, public Threadable
{
public:
    static V3API std::shared_ptr<World> getInstance();

    ~World();

	std::vector<SystemBase*> systems;
	std::unordered_map<unsigned int, std::vector<unsigned int>> components;
	std::vector<std::pair<unsigned int, std::vector<std::pair<unsigned int, unsigned int>>>*> entities;

    void onStartup() override;
    void tick() override;

    inline void onTick() override
    {
        if (!stepsAsync)
        {
            this->tick();
        }
    }

	inline std::pair<unsigned int, std::vector<std::pair<unsigned int, unsigned int>>>* entity_t(EntityHandle handle)
	{
		return (std::pair<unsigned int, std::vector<std::pair<unsigned int, unsigned int>>>*)handle;
	};

	inline std::vector<std::pair<unsigned int, unsigned int>>& getEntityRaw(EntityHandle handle)
	{
		return entity_t(handle)->second;
	};

	inline unsigned int getEntityIndex(EntityHandle handle)
	{
		return entity_t(handle)->first;
	};

	V3API EntityHandle makeEntity(ComponentBase* components, const unsigned int* componentTypes, size_t num);
	V3API void removeEntity(EntityHandle handle);

	template<class C>
	V3API void addComponent(EntityHandle handle, C* component);

	template<class C>
	V3API void removeComponent(EntityHandle handle);
	
	V3API void removeComponentInternal(unsigned int id, unsigned int index);

	inline void addSystem(SystemBase* system)
	{
		systems.push_back(system);
	};
protected:
    static std::shared_ptr<World> instance;
    World();
private:
    int stepsPerSecondTarget;
    bool stepsAsync = false;
    int stepThreadCount = 1;

    int stepsPerSecond;
    double stepAlpha;
    int stepCount;
    double lastStep;
    double stepTime;
    double deltaTime;
    double stepAccumulator;
    double stepPerformanceAccumulator;
};

struct WorldContainer : public World
{
	WorldContainer() : World() { }
};