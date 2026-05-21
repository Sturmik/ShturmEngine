#pragma once

#include "Types.h"
#include "Archetype.h"
#include "Component.h"

/////////////////////////////////////////////////////////////////////
// System
/////////////////////////////////////////////////////////////////////
// The system processes entities that contain a specific signature
/////////////////////////////////////////////////////////////////////
class System
{
public:
	System() = default;
	~System() = default;

	void AddArchetype(std::shared_ptr<Archetype> archetype);
	const std::vector<std::shared_ptr<Archetype>>& GetArchetypes() const;
	std::vector<std::shared_ptr<Archetype>>& AccessArchetypes();
	const Signature& GetComponentSignature() const;

	void GetFlatVector(std::vector<Entity>& outEntities);

	// Defines the component type that entities must have to be considered by the system
	template<typename TComponent>
	void RequireComponent();

private:
	Signature _componentSignature;
	std::vector<std::shared_ptr<Archetype>> _archetypes;
};

template<typename TComponent>
void System::RequireComponent()
{
	int componentId = Component<TComponent>::GetId();
	_componentSignature.set(componentId);
}
