#include "ECS.h"

int IComponent::nextId = 0;

Entity::Entity(int id, Registry* registry) : _id(id), _registry(registry)
{
}

void Entity::Kill()
{
    _registry->KillEntity(*this);
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

const std::vector<Entity>& System::GetSystemEntities() const
{
    return _entities;
}

std::vector<Entity>& System::AccessSystemEntities()
{
	return _entities;
}

const Signature& System::GetComponentSignature() const
{
	return _componentSignature;
}

void Registry::Update()
{
    // Processing the entities that are waiting to be created to the active Systems
    for (Entity entity : _entitiesToBeAdded)
    {
        // Add entity to according systems
        AddEntityToSystems(entity);
    }
    _entitiesToBeAdded.clear();

    // Processing the entities that are waiting to be killed from the active Systems
    for (Entity entity : _entitiesToBeKilled)
    {
        // Remove entities from all systems
        RemoveEntityFromSystems(entity);

        // Reset entity component signature, which is basically removing all components from it
        _entityComponentSignatures[entity.GetId()].reset();

        // Make the entity id available to be used
        _freeIds.push_back(entity.GetId());
    }
    _entitiesToBeKilled.clear();
}

Entity Registry::CreateEntity()
{
    int entityId;

    if (_freeIds.empty())
    {
        // If there are no free ids waiting to be reused
        entityId = _numEntities++;
    }
    else
    {
        // Reuse and id from the list of previously removed entities
        entityId = _freeIds.front();
        _freeIds.pop_front();
    }

    Entity entity(entityId, this);
    _entitiesToBeAdded.insert(entity);

    if (entityId >= _entityComponentSignatures.size())
    {
        _entityComponentSignatures.resize(entityId + 1);
    }

    LOG_INFO("Entity created with id = %d", entityId);

    return entity;
}

void Registry::KillEntity(Entity entity)
{
    _entitiesToBeKilled.insert(entity);
}

void Registry::AddEntityToSystems(Entity entity)
{
    const int entityId = entity.GetId();

    const Signature entityComponentSignature = _entityComponentSignatures[entityId];

    // Loop all systems
    for (std::pair<const std::type_index, std::shared_ptr<System>>& systemPair : _systems)
    {
        const Signature& systemComponentSignature = systemPair.second->GetComponentSignature();
        
        bool isInterested = (systemComponentSignature & entityComponentSignature) == systemComponentSignature;
    
        if (isInterested)
        {
            systemPair.second->AddEntityToSystem(entity);
        }
    }
}

void Registry::RemoveEntityFromSystems(Entity entity)
{
    // Loop all systems
    for (std::pair<const std::type_index, std::shared_ptr<System>>& systemPair : _systems)
    {
        systemPair.second->RemoveEntityFromSystem(entity);
    }
}