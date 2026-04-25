#pragma once

#include <bitset>
#include <vector>

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

	int GetId() const;

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

class Registry
{
	// TODO: ...
};