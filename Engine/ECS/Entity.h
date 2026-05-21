#pragma once

#include "Types.h"

class Registry;

class Entity
{
public:
	Entity() : _id(-1), _version(0), _registry(nullptr) {}
	Entity(int id, int version, Registry* registry);
	Entity(const Entity& entity) = default;

	void Kill();
	int GetId() const;
	uint32_t GetVersion() const;

	// Manage entity tags and groups
	void Tag(const std::string& tag);
	bool HasTag(const std::string& tag) const;
	void Group(const std::string& group);
	bool BelongsToGroup(const std::string& group) const;

	Entity& operator=(const Entity& other) = default;

	// Operator overloading for entity objects
	bool operator ==(const Entity& other) const { return _id == other._id && _version == other._version; }
	bool operator !=(const Entity& other) const { return _id != other._id && _version != other._version; }
	bool operator >(const Entity& other) const { return _id > other._id; }
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
	uint32_t _version;
};

namespace std
{
	template<>
	struct hash<Entity>
	{
		std::size_t operator()(const Entity& entity) const noexcept
		{
			return std::hash<int>()(entity.GetId()) ^ (std::hash<uint32_t>()(entity.GetVersion()) << 1);
		}
	};
}