#pragma once

#include "Types.h"
#include "Archetype.h"
#include "Component.h"

#include <functional>

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

	// Iterates over all archetypes in flat format 
	template<typename... Components, typename Func>
	void ForEach(Func&& func);

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

template<typename... Components, typename Func>
void System::ForEach(Func&& func)
{
	for (std::shared_ptr<Archetype>& archetype : _archetypes)
	{
		if ((archetype->signature & GetComponentSignature()) != GetComponentSignature())
		{
			continue;
		}

		if (archetype->entities.empty()) continue;

		std::tuple<Column<Components>*...> columns = {
			static_cast<Column<Components>*>(
				archetype->columns[Component<Components>::GetId()].get()
			)...
		};

		for (size_t i = 0; i < archetype->entities.size(); ++i)
		{
			Entity entity = archetype->entities[i];
			func(entity, std::get<Column<Components>*>(columns)->Get(i)...);
		}
	}
}