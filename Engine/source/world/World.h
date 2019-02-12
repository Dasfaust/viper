#include "../EngineExtension.h"
#include "../Threadable.h"
#include "Entity.h"

class World : public EngineExtension, public Threadable
{
public:
    World();
    ~World();
};