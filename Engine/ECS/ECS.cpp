#include "ECS.h"

#include "Logger/LoggerMacro.h"

int IComponent::nextId = 0;

Entity::Entity(int id) : _id(id)
{
}

int Entity::GetId() const
{
	return _id;
}

void System::AddEntityToSystem(Entity entity)
{
    auto it = std::find_if(_entities.begin(), _entities.end(),
        [&entity](const Entity& other) {
            return entity == other;
        });

    if (it == _entities.end())
    {
        _entities.emplace_back(entity);
    }
}

void System::RemoveEntityFromSystem(Entity entity)
{
    _entities.erase(
        std::remove_if(_entities.begin(), _entities.end(),
            [&entity](const Entity& other)
            {
                return entity == other;
            }),
        _entities.end()
    );
}

std::vector<Entity> System::GetSystemEntities() const
{
	return _entities;
}

const Signature& System::GetComponentSignature() const
{
	return _componentSignature;
}

Entity Registry::CreateEntity()
{
    int entityId;
    entityId = _numEntities++;

    Entity entity(entityId);
    _entitiesToBeAdded.insert(entity);

    LOG_INFO("Entity created with id = %d", entityId);

    return entity;
}

void Registry::Update()
{
    // TODO: Add the entities that are waiting to be created to the active Systems
    // TODO: Remove the entities that are waiting to be killed from the active Systems
}
