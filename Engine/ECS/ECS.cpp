#include "ECS.h"

#include "Events/KilLEntityEvent.h"

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

void Entity::Tag(const std::string& tag)
{
    _registry->TagEntity(*this, tag);
}

bool Entity::HasTag(const std::string& tag) const
{
    return _registry->EntityHasTag(*this, tag);
}

void Entity::Group(const std::string& group)
{
    _registry->GroupEntity(*this, group);
}

bool Entity::BelongsToGroup(const std::string& group) const
{
    return _registry->EntityBelongsToGroup(*this, group);
}

Registry& Entity::AccessRegistry()
{
    return *_registry;
}

void System::AddArchetype(std::shared_ptr<Archetype> archetype)
{
    // Verify that archetype signature matches system signature
    if ((archetype->signature & _componentSignature) != _componentSignature)
    {
        return;
    }

    // Check, if such archetype already exists
    for (std::shared_ptr<Archetype> existingArchetype : _archetypes)
    {
        if (existingArchetype == archetype)
        {
            return;
        }
    }

    // Add new archetype to the array
    _archetypes.push_back(archetype);
}

const std::vector<std::shared_ptr<Archetype>>& System::GetArchetypes() const
{
    return _archetypes;
}

std::vector<std::shared_ptr<Archetype>>& System::AccessArchetypes()
{
	return _archetypes;
}

const Signature& System::GetComponentSignature() const
{
	return _componentSignature;
}

void System::GetFlatVector(std::vector<Entity>& outEntities)
{
    for (std::shared_ptr<Archetype>& archetype : AccessArchetypes())
    {
        // Loop all entities that the system is interested in
        for (Entity& entity : archetype->entities)
        {
            outEntities.push_back(entity);
        }
    }
}

void Registry::Update()
{
    // Refreshes archetypes in systems in case of such need
    if (_bShouldRefreshSystemArchetypes)
    {
        RefreshSystemArchetypes();
    }

    // Processing the entities that are waiting to be killed from the active Systems
    for (Entity entity : _entitiesToBeKilled)
    {
        DestroyEntity(entity);
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

    if (_eventBusPtr)
    {
        _eventBusPtr->EmitEvent<KillEntityEvent>(entity.GetId());
    }
}

void Registry::TagEntity(Entity entity, const std::string& tag)
{
    _tagPerEntity.emplace(entity.GetId(), tag);
    _entityPerTag.emplace(tag, entity);
}

bool Registry::EntityHasTag(Entity entity, const std::string& tag) const
{
    auto tagPerEntity = _tagPerEntity.find(entity.GetId());

    if (tagPerEntity == _tagPerEntity.end())
    {
        return false;
    }

    return tagPerEntity->second == tag;
}

Entity Registry::GetEntityByTag(const std::string& tag) const
{
    auto entityPerTag = _entityPerTag.find(tag);

    if (entityPerTag == _entityPerTag.end())
    {
        return Entity();
    }

    return _entityPerTag.at(tag);
}

void Registry::RemoveEntityTag(Entity entity)
{
    auto tagPerEntity = _tagPerEntity.find(entity.GetId());

    if (tagPerEntity == _tagPerEntity.end())
    {
        return;
    }

    std::string tag = tagPerEntity->second;

    _tagPerEntity.erase(entity.GetId());
    _entityPerTag.erase(tag);
}

void Registry::GroupEntity(Entity entity, const std::string& group)
{
    _entitiesPerGroup[group].emplace(entity);
    _groupPerEntity[entity.GetId()] = group;
}

bool Registry::EntityBelongsToGroup(Entity entity, const std::string& group) const
{
    auto groupPerEntity = _groupPerEntity.find(entity.GetId());

    if (groupPerEntity == _groupPerEntity.end())
    {
        return false;
    }

    return groupPerEntity->second == group;
}

std::vector<Entity> Registry::GetEntitiesByGroup(const std::string& group) const
{
    if (_entitiesPerGroup.find(group) == _entitiesPerGroup.end())
    {
        return std::vector<Entity>();
    }

    std::set<Entity> setOfEntities = _entitiesPerGroup.at(group);
    return std::vector<Entity>(setOfEntities.begin(), setOfEntities.end());
}

void Registry::RemoveEntityGroup(Entity entity)
{
    auto groupPerEntity = _groupPerEntity.find(entity.GetId());

    if (groupPerEntity == _groupPerEntity.end())
    {
        return;
    }

    std::string group = groupPerEntity->second;

    _entitiesPerGroup[group].erase(entity);
    _groupPerEntity.erase(entity.GetId());
}

void Registry::RefreshSystemArchetypes()
{
    // Loop all systems
    for (std::pair<const std::type_index, std::shared_ptr<System>>& systemPair : _systems)
    {
        // Loop all archetypes
        for (std::pair<const Signature, std::shared_ptr<Archetype>>& archetype : _archetypes)
        {
            // Try to add new archetype
            systemPair.second->AddArchetype(archetype.second);
        }
    }

    _bShouldRefreshSystemArchetypes = false;
}

void Registry::ClearAll()
{
    _archetypes.clear();
    _locations.clear();
    _systems.clear();
    _entitiesToBeKilled.clear();
    _entityPerTag.clear();
    _tagPerEntity.clear();
    _entitiesPerGroup.clear();
    _groupPerEntity.clear();
    _freeIds.clear();
};

std::shared_ptr<Archetype> Registry::CreateOrGetArchetype(Signature archetypeSignature)
{
    // Find or create target archetype
    std::shared_ptr<Archetype>& targetArchetype = _archetypes[archetypeSignature];
    if (targetArchetype == nullptr)
    {
        // Initialize target archetype
        targetArchetype = std::make_shared<Archetype>();
        targetArchetype->signature = archetypeSignature;
        LOG_INFO("New archetype created: %s", archetypeSignature.to_string().c_str());

        // Initialize target archetype columns according to signature
        targetArchetype->columns.resize(MAX_COMPONENTS);

        // Mark the need to update system archetypes
        _bShouldRefreshSystemArchetypes = true;
    }

    // Ensure all required columns exist
    for (int i = 0; i < MAX_COMPONENTS; ++i)
    {
        if (archetypeSignature.test(i) && targetArchetype->columns[i] == nullptr)
        {
            targetArchetype->columns[i] = CreateColumn(i);
        }
    }

    return targetArchetype;
}

void Registry::RemoveEntityFromArchetype(std::shared_ptr<Archetype> archetype, uint32_t row)
{
    if (archetype->entities.empty() || row >= archetype->entities.size()) 
    {
        return;
    }

    // Get last entity id in archetype
    Entity lastEntity = archetype->entities[archetype->entities.size() - 1];

    // Swap last element with old element location
    archetype->entities[row] = lastEntity;
    for (int i = 0; i < archetype->columns.size(); ++i)
    {
        if (archetype->columns[i] != nullptr)
        {
            archetype->columns[i]->RemoveSwapLast(row);
        }
    }
    archetype->entities.pop_back();

    // Update last entity location
    _locations[lastEntity].row = row;
}

void Registry::MoveEntity(Location& oldLocation, std::shared_ptr<Archetype> newArchetype, uint32_t newRow)
{
    // Get old archetype
    std::shared_ptr<Archetype> oldArchetype = oldLocation.archetype; 

    // Get old row
    uint32_t oldRow = oldLocation.row;

    // Copy all components that existed in old archetype to the new one
    for (int i = 0; i < oldArchetype->columns.size(); ++i)
    {
        if (oldArchetype->signature.test(i) &&
            newArchetype->signature.test(i) &&
            oldArchetype->columns[i] &&
            newArchetype->columns[i])
        {
            newArchetype->columns[i]->CopyFrom(oldArchetype->columns[i].get(), oldRow, newRow);
        }
    }

    // Remove entity from archetype
    RemoveEntityFromArchetype(oldArchetype, oldRow);

    // Update old location with new archetype and row
    oldLocation.archetype = newArchetype;
    oldLocation.row = newRow;
}

void Registry::DestroyEntity(Entity entity)
{
    auto it = _locations.find(entity);
    if (it == _locations.end())
    {
        return;
    }

    Location& location = it->second;

    // Remove entity from archetypes
    RemoveEntityFromArchetype(location.archetype, location.row);

    // Clean up tracking
    _locations.erase(entity);
    _entityComponentSignatures[entity.GetId()].reset();

    // Return ID to pool
    _freeIds.push_back(entity.GetId());

    LOG_INFO("Entity %d fully destroyed", entity.GetId());
}

std::unique_ptr<IColumn> Registry::CreateColumn(int componentId)
{
    auto it = _componentColumnCreators.find(componentId);
    if (it != _componentColumnCreators.end())
    {
        return it->second();
    }

    LOG_ERROR("No column creator registered for component ID %d", componentId);
    return nullptr;
}