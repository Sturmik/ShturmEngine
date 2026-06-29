#pragma once

#include "ECS/ECS.h"
#include "Components/ScriptComponent.h"
#include "Components/TransformComponent.h"

// Declare some native C++ functions that we will bind with Lua functions
static void GetEntityPosition(Entity entity)
{

}

static void SetEntityPosition(Entity entity, float x, float y)
{
	if (entity.HasComponent<TransformComponent>())
	{
		TransformComponent& transform = entity.GetComponent<TransformComponent>();
		transform.position.x = x;
		transform.position.y = y;
	}
	else
	{
		LOG_ERROR("Trying to set the poisition of an entity that has no transform component");
	}
}

class ScriptSystem : public System
{
public:
	ScriptSystem()
	{
		RequireComponent<ScriptComponent>();
	}

	void CreateLuaBindings(sol::state& luaState)
	{
		// Create the "entity" usertype, so Lua knows what an entity is
		luaState.new_usertype<Entity>(
		"entity",
		"get_id", &Entity::GetId,
		"destroy", &Entity::Kill,
		"has_tag", &Entity::HasTag,
		"belongs_to_group", &Entity::BelongsToGroup
		);

		// Create all bindings between C++ and Lua functions
		luaState.set_function("set_position", SetEntityPosition);
	}

	void Update(float deltaTime, int ellapsedTime)
	{
		// Loop all entities that have a script component and invoke their Lua function
		ForEach<ScriptComponent>([&](Entity entity, const ScriptComponent& script)
		{
			// Invoke sol::function
			script.function(entity, deltaTime, ellapsedTime);
		});
	}
};