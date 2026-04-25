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
}

void System::RemoveEntityFromSystem(Entity entity)
{
}

std::vector<Entity> System::GetSystemEntities() const
{
	return _entities;
}

Signature System::GetComponentSignature() const
{
	return _componentSignature;
}
