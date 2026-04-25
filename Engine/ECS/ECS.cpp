#include "ECS.h"

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
            return entity.GetId() == other.GetId();
        });

    if (it == _entities.end())
    {
        _entities.push_back(entity);
    }
}

void System::RemoveEntityFromSystem(Entity entity)
{
    _entities.erase(
        std::remove_if(_entities.begin(), _entities.end(),
            [&entity](const Entity& other)
            {
                return entity.GetId() == other.GetId();
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
