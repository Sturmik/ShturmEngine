#pragma once

#include <bitset>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <set>
#include <memory>

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
	// Returns the unique id of Component<T>
	static int GetId()
	{
		static int id = nextId++;
		return id;
	}
};

class Entity
{
public:
	Entity(int id);
	Entity(const Entity& entity) = default;

	int GetId() const;

	Entity& operator=(const Entity& other) = default;

	bool operator ==(const Entity& other) const { return _id == other._id; }
	bool operator !=(const Entity& other) const { return _id != other._id; }
	bool operator >(const Entity & other) const { return _id > other._id; }
	bool operator <(const Entity& other) const { return _id < other._id; }

private:
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
	std::vector<Entity> GetSystemEntities() const;
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
};

template<typename T>
class Pool : public IPool
{
public:
	Pool(int size = 100)
	{
		_data.resize(size);
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

	void Set(int index, T object)
	{
		_data[index] = object;
	}

	T& Get(int index)
	{
		return static_cast<T&>(_data[index]);
	}

	T& operator[] (unsigned int index)
	{
		return _data[index];
	}

private:
	std::vector<T> _data;
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
	Registry() = default;

	void Update();

	// Entity management

	Entity CreateEntity();
	
	void AddEntityToSystem(Entity entity);

	// Component management

	template<typename TComponent, typename ...TArgs>
	void AddComponent(Entity entity, TArgs&& ...args);
	template<typename TComponent>
	void RemoveComponent(Entity entity);
	template<typename TComponent>
	bool HasComponent(Entity entity) const;

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
	void AddEntityToSystems(Entity entity);

private:
	int _numEntities;

	// Vector of component pools, each pool contains all the data for a certain component type
	// [Vector index = component type id]
	// [Pool index = entity id]
	std::vector<IPool*> _componentPools;

	// Vector of component signatures per entity, saying which component is turned "on" for a given entity
	// [Vector index = entity id]
	std::vector<Signature> _entityComponentSignatures;

	std::unordered_map<std::type_index, System*> _systems;

	// Set of entities that are flagged to be added or removed in the next registry Update()
	std::set<Entity> _entitiesToBeAdded;
	std::set<Entity> _entitiesToBeKilled;
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
		Pool<TComponent>* newComponentPool = new Pool<TComponent>();
		_componentPools[componentId] = newComponentPool;
	}

	// Get the pool of component values for that component type
	Pool<TComponent>* componentPool = Pool<TComponent>(_componentPools[componentId]);

	// If the entity id is greater than the current size of the component pool, then resize the pool
	if (entityId >= componentPool->GetSize())
	{
		componentPool->Resize(_numEntities);
	}

	// Create a new Component object of the type T, and forward the various parameters to the constructor
	TComponent newComponent(std::forward<TArgs>(args)...);

	// Add the new component to the component pool list, using the entity id as index
	componentPool->Set(entityId, newComponent);

	// Change the component signature of the entity and set the component id on the bitset to 1
	_entityComponentSignatures[entityId].set(componentId);
}

template<typename TComponent>
void Registry::RemoveComponent(Entity entity)
{
	const int componentId = Component<TComponent>::GetId();
	const int entityId = entity.GetId();

	_entityComponentSignatures[entityId].set(componentId, false);
}

template<typename TComponent>
bool Registry::HasComponent(Entity entity) const
{
	const int componentId = Component<TComponent>::GetId();
	const int entityId = entity.GetId();

	return _entityComponentSignatures[entityId].test(componentId);
}

template<typename TSystem, typename ...TArgs>
void Registry::AddSystem(TArgs && ...args)
{
	TSystem* newSystem = new TSystem(std::forward(args));
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
