#pragma once
#include "../Tickable.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

class RenderCommand : public Tickable
{
public:
    struct WorldState
    {
        glm::vec3 _worldCoordinates = glm::vec3();
        glm::vec3 interpCoordinates = glm::vec3();
        glm::vec3 _scale = glm::vec3(1.0f);
        float _rotationX = 0.0f;
        float _rotationY = 0.0f;
        float _rotationZ = 0.0f;

        glm::vec3 worldCoordinates = glm::vec3();
        glm::vec3 scale = glm::vec3(1.0f);
        float rotationX = 0.0f;
        float rotationY = 0.0f;
        float rotationZ = 0.0f;
    };

    unsigned int index = 0;

    struct LocalObject
    {
        unsigned int id;
        std::string mesh;
        std::string model;
        std::string texture;
        int textureSlot;
        std::string shader;
        tbb::concurrent_vector<WorldState> instances;
    };

    tbb::concurrent_unordered_map<unsigned int, std::shared_ptr<LocalObject>> objects;

    inline unsigned int addObject(
        std::string mesh = "",
        std::string model = "",
        std::string texture = "",
        int textureSlot = 0,
        std::string shader = "basic",
        std::vector<WorldState> inst = { })
    {
        unsigned int id = index += 1;
        std::shared_ptr<LocalObject> object = std::make_shared<LocalObject>();
        object->id = id;
        object->mesh = mesh;
        object->model = model;
        object->texture = texture;
        object->textureSlot = textureSlot;
        object->shader = shader;
        decltype(object->instances) instances;
        for (WorldState instance : inst)
        {
            instances.push_back(instance);
        }
        object->instances = instances;
        objects[id] = object;

        onObjectAdd(id);

        return id;
    };

    inline std::shared_ptr<LocalObject> getObject(unsigned int id)
    {
        return objects[id];
    };

    inline void removeObject(unsigned int id)
    {
        decltype(objects) _objects;
        for (auto&& o : objects)
        {
            if (o.first != id)
            {
                _objects[id] = o.second;
            }
        }
        objects.swap(_objects);
    };

    virtual void onObjectAdd(unsigned int id) { };

    virtual void onPreRender() { };

    virtual void onPostRender() { };
};