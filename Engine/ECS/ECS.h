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

class Component
{
	// TODO: ...
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
	Signature GetComponentSignature() const;

private:
	Signature _componentSignature;
	std::vector<Entity> _entities;
};

class Registry
{
	// TODO: ...
};