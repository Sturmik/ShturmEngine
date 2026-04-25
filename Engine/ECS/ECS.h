#pragma once

#include <bitset>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <set>

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

	Entity CreateEntity();
	
	void AddEntityToSystem(Entity entity);

	void Update();

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