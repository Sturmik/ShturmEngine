#pragma once

#include "Types.h"
#include "Component.h"
#include "Column.h"
#include "Archetype.h"
#include "Entity.h"
#include "System.h"

#include "EventBus/EventBus.h"

#include <functional>
#include <unordered_map>
#include <set>
#include <deque>

/////////////////////////////////////////////////////////////////////
// Registry
/////////////////////////////////////////////////////////////////////
// The registry manages the creation of entities, add systems,
// and components.
/////////////////////////////////////////////////////////////////////
class Registry
{
public:
	Registry();
	~Registry();

	void Update();

	// Entity management

	Entity CreateEntity();
	void KillEntity(Entity entity);

	// Tag management
	void TagEntity(Entity entity, const std::string& tag);
	bool EntityHasTag(Entity entity, const std::string& tag) const;
	Entity GetEntityByTag(const std::string& tag) const;
	void RemoveEntityTag(Entity entity);

	// Group management
	void GroupEntity(Entity entity, const std::string& group);
	bool EntityBelongsToGroup(Entity entity, const std::string& group) const;
	std::vector<Entity> GetEntitiesByGroup(const std::string& group) const;
	void RemoveEntityGroup(Entity entity);

	// Component management

	template<typename TComponent, typename ...TArgs>
	void AddComponent(Entity entity, TArgs&& ...args);
	template<typename TComponent>
	void RemoveComponent(Entity entity);
	template<typename TComponent>
	bool HasComponent(Entity entity) const;
	template<typename TComponent>
	TComponent& GetComponent(Entity entity) const;

	// System management

	template<typename TSystem, typename ...TArgs>
	void AddSystem(TArgs&& ...args);
	template<typename TSystem>
	void RemoveSystem();
	template<typename TSystem>
	bool HasSystem() const;
	template<typename TSystem>
	TSystem& GetSystem() const;

	// Refreshes system archetypes and adds new ones
	void RefreshSystemArchetypes();

	// Sets event bus for handling callbacks
	void SetEventBus(EventBus* eventBusPtr)
	{
		_eventBusPtr = eventBusPtr;
	}

	void ClearAll();

private:
	// Ensure component registration
	template<typename TComponent>
	void EnsureComponentRegistered();

	// Fully destroyes entity from the system
	void DestroyEntity(Entity entity);

	// Logic for creating columns with specific components
	std::unique_ptr<IColumn> CreateColumn(int componentId);

	// Archetype logic
	std::shared_ptr<Archetype> CreateOrGetArchetype(Signature archetypeSignature);
	void RemoveEntityFromArchetype(std::shared_ptr<Archetype> archetype, uint32_t row);
	void MoveEntity(Location& oldLocation, std::shared_ptr<Archetype> newArchetype, uint32_t newRow);

	// Component column factory
	using ColumnCreateFunc = std::function<std::unique_ptr<IColumn>()>;
	std::unordered_map<int, ColumnCreateFunc> _componentColumnCreators;

	int _numEntities;

	// Archetypes are mapped by component signatures
	std::unordered_map<Signature, std::shared_ptr<Archetype>> _archetypes;

	// Map entity to according location
	std::unordered_map<Entity, Location> _locations;

	// Used as a control variable to define whether system archetype refresh is required
	bool _bShouldRefreshSystemArchetypes;

	// Vector of component signatures per entity, saying which component is turned "on" for a given entity
	// [Vector index = entity id]
	std::vector<Signature> _entityComponentSignatures;

	// Systems unordered map
	std::unordered_map<std::type_index, std::shared_ptr<System>> _systems;

	// Set of entities that are flagged to be added or removed in the next registry Update()
	std::set<Entity> _entitiesToBeKilled;

	// Entity tags (one tag name per entity)
	std::unordered_map<std::string, Entity> _entityPerTag;
	std::unordered_map<int, std::string> _tagPerEntity;

	// Entity groups (a set of entities per group name)
	std::unordered_map<std::string, std::set<Entity>> _entitiesPerGroup;
	std::unordered_map<int, std::string> _groupPerEntity;

	// Deque of free ids that were previously removed
	std::deque<int> _freeIds;

	// Reference to event bus
	EventBus* _eventBusPtr;
};

#include "Registry.inl"