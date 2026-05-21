#include "System.h"

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