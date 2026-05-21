#include "Entity.h"

#include "Registry.h"

Entity::Entity(int id, int version, Registry* registry) : _id(id), _version(version), _registry(registry)
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

uint32_t Entity::GetVersion() const
{
    return _version;
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