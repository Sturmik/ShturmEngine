#pragma once

#include <bitset>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <set>
#include <memory>
#include <deque>
#include <functional>

#include "EventBus/EventBus.h"

const unsigned int MAX_COMPONENTS = 32;
/////////////////////////////////////////////////////////////////////
// Signature
/////////////////////////////////////////////////////////////////////
// We use a bitset (1s and 0s) to keep track of which components
// and entity has and also helps keep track of which entities
// a system is interested in.
/////////////////////////////////////////////////////////////////////
typedef std::bitset<MAX_COMPONENTS> Signature;

struct IComponent
{
protected:
	static int nextId;
};

// Used to assign a unique id to a component type
template<typename T>
class Component : public IComponent
{
public:
	// Returns the unique id of Component<T>
	static int GetId()
	{
		static int id = nextId++;
		return id;
	}
};

class Registry;

class Entity
{
public:
	Entity() : _id(-1), _registry(nullptr) {}
	Entity(int id, Registry* registry);
	Entity(const Entity& entity) = default;
	
	void Kill();
	int GetId() const;

	// Manage entity tags and groups
	void Tag(const std::string& tag);
	bool HasTag(const std::string& tag) const;
	void Group(const std::string& group);
	bool BelongsToGroup(const std::string& group) const;

	Entity& operator=(const Entity& other) = default;

	// Operator overloading for entity objects
	bool operator ==(const Entity& other) const { return _id == other._id; }
	bool operator !=(const Entity& other) const { return _id != other._id; }
	bool operator >(const Entity & other) const { return _id > other._id; }
	bool operator <(const Entity& other) const { return _id < other._id; }

	// Manage entity components
	template<typename TComponent, typename ...TArgs>
	void AddComponent(TArgs&& ...args);
	template<typename TComponent>
	void RemoveComponent();
	template<typename TComponent>
	bool HasComponent() const;
	template<typename TComponent>
	TComponent& GetComponent() const;

	Registry& AccessRegistry();

private:
	// Hold a pointer to the entity's owner registry
	Registry* _registry;

	int _id;
};

namespace std
{
	template<>
	struct hash<Entity>
	{
		std::size_t operator()(const Entity& entity) const noexcept
		{
			return std::hash<int>()(entity.GetId());
		}
	};
}

struct Archetype;

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

// Column is used to define array of components
struct IColumn
{
	virtual ~IColumn() = default;
	virtual void RemoveSwapLast(uint32_t index) = 0;

	virtual void CopyFrom(IColumn* source, uint32_t srcIndex, uint32_t dstIndex) = 0;

	// Allows outter system to identify component type
	virtual std::unique_ptr<IColumn> CloneEmpty() const = 0;
};

template<typename T>
struct Column : public IColumn
{
	std::vector<T> data;
	
	void RemoveSwapLast(uint32_t index) override
	{
		data[index] = std::move(data.back());
		data.pop_back();
	}

	void Push(const T& value)
	{
		data.push_back(value);
	}

	T& Get(uint32_t index)
	{
		return data[index];
	}

	void CopyFrom(IColumn* source, uint32_t sourceIndex, uint32_t destinationIndex) override
	{
		Column<T>* sourceColumn = static_cast<Column<T>*>(source);
		if (data.size() <= destinationIndex) 
		{		
			data.resize(destinationIndex + 1);
		}

		data[destinationIndex] = sourceColumn->data[sourceIndex];
	}

	std::unique_ptr<IColumn> CloneEmpty() const override
	{
		return std::make_unique<Column<T>>();
	}
};

/////////////////////////////////////////////////////////////////////
// Archetype
/////////////////////////////////////////////////////////////////////
// An archetype represents a unique combination of component types.
//
// Example:
//
// Archetype A:
// [Transform, Velocity]
//
// Archetype B:
// [Transform, Sprite]
//
// Archetype C:
// [Transform, Velocity, Health]
//
// Every entity belongs to exactly ONE archetype at a time,
// depending on which components it currently owns.
//
// ------------------------------------------------------------------
// Why archetypes exist
// ------------------------------------------------------------------
//
// Archetypes store entities that have the same component layout
// together in tightly packed contiguous arrays.
//
// This allows systems to iterate cache-friendly blocks of memory:
//
// Instead of:
//
//   Position of entity 1 somewhere in memory
//   Position of entity 2 somewhere else
//   Position of entity 3 somewhere else
//
// we get:
//
//   [P1][P2][P3][P4][P5]
//
// packed together sequentially.
//
// This drastically improves:
// - CPU cache locality
// - iteration speed
// - SIMD/vectorization opportunities
// - large-scale ECS performance
// 
// ------------------------------------------------------------------
// Archetype layout
// ------------------------------------------------------------------
//
// Each archetype acts similarly to a database table:
//
// -----------------------------------------------------
// | Entity | Transform | Velocity | Health |
// -----------------------------------------------------
// |   1    |    ...    |    ...   |   ...  |
// |   2    |    ...    |    ...   |   ...  |
// -----------------------------------------------------
//
// Each component type is stored in its own dense column.
/////////////////////////////////////////////////////////////////////
struct Archetype
{
	// Component set
	Signature signature; 

	// Rows
	std::vector<Entity> entities;

	// Component array
	// [componentId]->[componentArray]
	std::vector<std::unique_ptr<IColumn>> columns;
};

// location of element in archetype
struct Location
{
	std::shared_ptr<Archetype> archetype;
	uint32_t row;
};

/////////////////////////////////////////////////////////////////////
// Registry
/////////////////////////////////////////////////////////////////////
// The registry manages the creation of entities, add systems,
// and components.
/////////////////////////////////////////////////////////////////////
class Registry
{
public: 
	Registry() : _numEntities(0), _eventBusPtr(nullptr), _bShouldRefreshSystemArchetypes(false) { LOG_INFO("Registry constructor called!"); }
	~Registry() { LOG_INFO("Registry destructor called!"); }

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

template<typename TComponent>
void Registry::EnsureComponentRegistered()
{
	static bool registered = false;
	if (registered)
	{
		return;
	}

	int id = Component<TComponent>::GetId();
	_componentColumnCreators[id] = []() {
		return std::make_unique<Column<TComponent>>();
	};
	registered = true;

	LOG_INFO("ECS system registered new component for column creation: %i id", id);
}

template<typename TComponent, typename ...TArgs>
void Registry::AddComponent(Entity entity, TArgs&& ...args)
{
	EnsureComponentRegistered<TComponent>();

	const int entityId = entity.GetId();
	const int componentId = Component<TComponent>::GetId();

	Location& oldLocation = _locations[entity];
	Signature oldSignature = oldLocation.archetype ? oldLocation.archetype->signature : Signature();

	// Build new signature
	Signature newSignature = oldSignature;
	newSignature.set(componentId);

	// Find or create target archetype
	std::shared_ptr<Archetype> targetArchetype = CreateOrGetArchetype(newSignature);
	
	// Create new row in target archetype
	uint32_t newRow = targetArchetype->entities.size();
	targetArchetype->entities.push_back(entity);

	Column<TComponent>* column = static_cast<Column<TComponent>*>(targetArchetype->columns[componentId].get());
	column->Push(TComponent(std::forward<TArgs>(args)...));

	// If entity already exists - move it
	if (oldLocation.archetype)
	{
		MoveEntity(oldLocation, targetArchetype, newRow);
	}
	else
	{
		oldLocation.archetype = targetArchetype;
		oldLocation.row = newRow;
	}

	// Update entity component signature
	_entityComponentSignatures[entityId] = newSignature;

	LOG_INFO("Component id = %d was added to entity id %d", componentId, entityId);
}

template<typename TComponent>
void Registry::RemoveComponent(Entity entity)
{
	const int entityId = entity.GetId();
	const int componentId = Component<TComponent>::GetId();

	Location& oldLocation = _locations[entity];
	Signature oldSignature = oldLocation.archetype ? oldLocation.archetype->signature : Signature();

	// Build new signature
	Signature newSignature = oldSignature;
	// Remove bit for old component
	newSignature.set(componentId, false);

	// Find or create target archetype
	std::shared_ptr<Archetype> targetArchetype = CreateOrGetArchetype(newSignature);

	// Create new row in target archetype
	uint32_t newRow = targetArchetype->entities.size();
	targetArchetype->entities.push_back(entity);

	// If entity already exists - move it
	if (oldLocation.archetype)
	{
		MoveEntity(oldLocation, targetArchetype, newRow);
	}
	else
	{
		oldLocation.archetype = targetArchetype;
		oldLocation.row = newRow;
	}

	// Set this component signature for that entity to false
	_entityComponentSignatures[entityId].set(componentId, false);

	LOG_INFO("Component id = %d was removed from entity id %d", componentId, entityId);
}

template<typename TComponent>
bool Registry::HasComponent(Entity entity) const
{
	const int componentId = Component<TComponent>::GetId();
	const int entityId = entity.GetId();

	return _entityComponentSignatures[entityId].test(componentId);
}

template<typename TComponent>
TComponent& Registry::GetComponent(Entity entity) const
{
	const int componentId = Component<TComponent>::GetId();

	// Get location and component index
	const Location& location = _locations.at(entity);
	int componentIndex = location.row;

	// Get according column type
	Column<TComponent>* column = static_cast<Column<TComponent>*>(location.archetype->columns[componentId].get());

	return column->Get(componentIndex);
}

template<typename TSystem, typename ...TArgs>
void Registry::AddSystem(TArgs && ...args)
{
	std::shared_ptr<TSystem> newSystem = std::make_shared<TSystem>(std::forward(args)...);
	_systems.insert(std::make_pair(std::type_index(typeid(TSystem)), newSystem));
}

template<typename TSystem>
void Registry::RemoveSystem()
{
	std::type_index system = _systems.find(std::type_index(typeid(TSystem)));
	_systems.erase(system);
}

template<typename TSystem>
bool Registry::HasSystem() const
{
	return _systems.find(std::type_index(typeid(TSystem))) != _systems.end();
}

template<typename TSystem>
TSystem& Registry::GetSystem() const
{
	auto system = _systems.find(std::type_index(typeid(TSystem)));
	return *(std::static_pointer_cast<TSystem>(system->second));
}

template<typename TComponent, typename ...TArgs>
void Entity::AddComponent(TArgs && ...args)
{
	_registry->AddComponent<TComponent>(*this, std::forward<TArgs>(args)...);
}

template<typename TComponent>
void Entity::RemoveComponent()
{
	_registry->RemoveComponent<TComponent>(*this);
}

template<typename TComponent>
bool Entity::HasComponent() const
{
	return _registry->HasComponent<TComponent>(*this);
}

template<typename TComponent>
TComponent& Entity::GetComponent() const
{
	return _registry->GetComponent<TComponent>(*this);
}
