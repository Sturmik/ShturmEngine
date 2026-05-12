#pragma once

#include <bitset>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <set>
#include <memory>
#include <deque>

#include "Logger/LoggerMacro.h"

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

	void AddEntityToSystem(Entity entity);
	void RemoveEntityFromSystem(Entity entity);
	const std::vector<Entity>& GetSystemEntities() const;
	std::vector<Entity>& AccessSystemEntities();
	const Signature& GetComponentSignature() const;

	// Defines the component type that entities must have to be considered by the system
	template<typename TComponent>
	void RequireComponent();

private:
	Signature _componentSignature;
	std::vector<Entity> _entities;
};

template<typename TComponent>
void System::RequireComponent()
{
	int componentId = Component<TComponent>::GetId();
	_componentSignature.set(componentId);
}

/////////////////////////////////////////////////////////////////////
// Pool
/////////////////////////////////////////////////////////////////////
// A pool is just a vector (contigious data) of objects of type T
/////////////////////////////////////////////////////////////////////
class IPool
{
public:
	virtual ~IPool() {}

	virtual void Remove(int entityId) = 0;
};

template<typename T>
class Pool : public IPool
{
public:
	Pool(int size = 100)
	{
		_data.reserve(size);
	}

	virtual ~Pool() = default;

	bool IsEmpty() const
	{
		return _data.empty();
	}

	int GetSize() const
	{
		return _data.size();
	}

	void Resize(int n)
	{
		_data.resize(n);
	}

	void Clear()
	{
		_data.clear();
	}

	void Add(T object)
	{
		_data.push_back(object);
	}

	void Set(int entityId, T object)
	{
		if (_entityIdToIndex.find(entityId) != _entityIdToIndex.end())
		{
			// If the element already exists, simply replace the component object
			int index = _entityIdToIndex[entityId];
			_data[index] = object;
		}
		else
		{
			// When adding a new object, we keep track of the entity ids and their vector index
			_entityIdToIndex.emplace(entityId, _data.size());
			_indexToEntityId.emplace(_data.size(), entityId);
			_data.push_back(object);
		}
	}

	virtual void Remove(int entityId) override
	{
		if (_entityIdToIndex.find(entityId) == _entityIdToIndex.end())
		{
			return;
		}

		// Copy the last element to the deleted position to keep the array packed
		int indexOfRemoved = _entityIdToIndex[entityId];
		int indexOfLast = _data.size() - 1;
		_data[indexOfRemoved] = _data[indexOfLast];

		// Update the index entity maps to point to the correct elements
		int entityIdOfLastElement = _indexToEntityId[indexOfLast];
		_entityIdToIndex[entityIdOfLastElement] = indexOfRemoved;
		_indexToEntityId[indexOfRemoved] = entityIdOfLastElement;

		_entityIdToIndex.erase(entityId);
		_indexToEntityId.erase(indexOfLast);

		_data.resize(_data.size() - 1);
	}

	T& Get(int entityId)
	{
		int index = _entityIdToIndex[entityId];
		return static_cast<T&>(_data[index]);
	}

	T& operator[] (unsigned int entityId)
	{
		return Get(entityId);
	}

private:
	std::vector<T> _data;

	// Helper maps to keep track of entity ids per index, so the vector is always packed
	std::unordered_map<int, int> _entityIdToIndex;
	std::unordered_map<int, int> _indexToEntityId;
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
	Registry() : _numEntities(0) { LOG_INFO("Registry constructor called!"); }
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

	// Checks the component signature of an entity and add the entity to the systems that are interested in it
	// Add or remove entities from their systems
	void AddEntityToSystems(Entity entity);
	void RemoveEntityFromSystems(Entity entity);

private:
	int _numEntities;

	// Vector of component pools, each pool contains all the data for a certain component type
	// [Vector index = component type id]
	// [Pool index = entity id]
	std::vector<std::shared_ptr<IPool>> _componentPools;

	// Vector of component signatures per entity, saying which component is turned "on" for a given entity
	// [Vector index = entity id]
	std::vector<Signature> _entityComponentSignatures;

	// Systems unordered map
	std::unordered_map<std::type_index, std::shared_ptr<System>> _systems;

	// Set of entities that are flagged to be added or removed in the next registry Update()
	std::set<Entity> _entitiesToBeAdded;
	std::set<Entity> _entitiesToBeModified;
	std::set<Entity> _entitiesToBeKilled;

	// Entity tags (one tag name per entity)
	std::unordered_map<std::string, Entity> _entityPerTag;
	std::unordered_map<int, std::string> _tagPerEntity;

	// Entity groups (a set of entities per group name)
	std::unordered_map<std::string, std::set<Entity>> _entitiesPerGroup;
	std::unordered_map<int, std::string> _groupPerEntity;

	// Deque of free ids that were previously removed
	std::deque<int> _freeIds;
};

template<typename TComponent, typename ...TArgs>
void Registry::AddComponent(Entity entity, TArgs&& ...args)
{
	const int componentId = Component<TComponent>::GetId();
	const int entityId = entity.GetId();

	// If the component id is greater than the current size of the componentPools, then resize the vector
	if (componentId >= _componentPools.size())
	{
		_componentPools.resize(componentId + 1, nullptr);
	}

	// If we still don't have a Pool for that component type
	if (_componentPools[componentId] == nullptr)
	{
		std::shared_ptr<Pool<TComponent>> newComponentPool = std::make_shared<Pool<TComponent>>();
		_componentPools[componentId] = newComponentPool;
	}

	// Get the pool of component values for that component type
	std::shared_ptr<Pool<TComponent>> componentPool = std::static_pointer_cast<Pool<TComponent>>(_componentPools[componentId]);

	// Create a new Component object of the type T, and forward the various parameters to the constructor
	TComponent newComponent(std::forward<TArgs>(args)...);

	// Add the new component to the component pool list, using the entity id as index
	componentPool->Set(entityId, newComponent);

	// Change the component signature of the entity and set the component id on the bitset to 1
	_entityComponentSignatures[entityId].set(componentId);

	LOG_INFO("Component id = %d was added to entity id %d", componentId, entityId);
}

template<typename TComponent>
void Registry::RemoveComponent(Entity entity)
{
	const int componentId = Component<TComponent>::GetId();
	const int entityId = entity.GetId();

	// Set this component signature for that entity to false
	_entityComponentSignatures[entityId].set(componentId, false);

	// Get the pool of component values for that component type
	std::shared_ptr<Pool<TComponent>> componentPool = std::static_pointer_cast<Pool<TComponent>>(_componentPools[componentId]);
	componentPool->Remove(entityId);

	_entitiesToBeModified.insert(entity);

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
	const int entityId = entity.GetId();

	std::shared_ptr<Pool<TComponent>> componentPool = std::static_pointer_cast<Pool<TComponent>>(_componentPools[componentId]);

	return componentPool->Get(entityId);
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
