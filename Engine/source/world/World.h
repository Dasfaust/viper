#include "../EngineExtension.h"
#include "../Threadable.h"

class World : public EngineExtension, public Threadable
{
public:
    World();
    ~World();
};