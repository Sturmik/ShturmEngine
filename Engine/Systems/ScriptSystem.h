#pragma once

#include "ECS/ECS.h"
#include "Components/ScriptComponent.h"
#include "Components/TransformComponent.h"

static std::tuple<float, float> GetEntityPosition(Entity entity) 
{
    if (entity.HasComponent<TransformComponent>()) 
    {
        const TransformComponent transform = entity.GetComponent<TransformComponent>();
        return std::make_tuple(transform.position.x, transform.position.y);
    }
    else 
    {
        LOG_ERROR("Trying to get the position of an entity that has no transform component");
        return std::make_tuple(0.0, 0.0);
    }
}

static std::tuple<float, float> GetEntityVelocity(Entity entity)
{
    if (entity.HasComponent<RigidBodyComponent>()) 
    {
        const RigidBodyComponent rigidbody = entity.GetComponent<RigidBodyComponent>();
        return std::make_tuple(rigidbody.velocity.x, rigidbody.velocity.y);
    }
    else 
    {
        LOG_ERROR("Trying to get the velocity of an entity that has no rigidbody component");
        return std::make_tuple(0.0, 0.0);
    }
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
        LOG_ERROR("Trying to set the position of an entity that has no transform component");
    }
}

static void SetEntityVelocity(Entity entity, float x, float y)
{
    if (entity.HasComponent<RigidBodyComponent>()) 
    {
        RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();
        rigidbody.velocity.x = x;
        rigidbody.velocity.y = y;
    }
    else {
        LOG_ERROR("Trying to set the velocity of an entity that has no rigidbody component");
    }
}

static void SetEntityRotation(Entity entity, float angle)
{
    if (entity.HasComponent<TransformComponent>()) 
    {
        TransformComponent& transform = entity.GetComponent<TransformComponent>();
        transform.rotation = angle;
    }
    else 
    {
        LOG_ERROR("Trying to set the rotation of an entity that has no transform component");
    }
}

static void SetEntityAnimationFrame(Entity entity, int frame)
{
    if (entity.HasComponent<AnimationComponent>()) 
    {
        AnimationComponent& animation = entity.GetComponent<AnimationComponent>();
        animation.currentFrame = frame;
    }
    else 
    {
        LOG_ERROR("Trying to set the animation frame of an entity that has no animation component");
    }
}

static void SetProjectileVelocity(Entity entity, float x, float y)
{
    if (entity.HasComponent<ProjectileEmitterComponent>()) 
    {
        ProjectileEmitterComponent& projectileEmitter = entity.GetComponent<ProjectileEmitterComponent>();
        projectileEmitter.projectileVelocity.x = x;
        projectileEmitter.projectileVelocity.y = y;
    }
    else 
    {
        LOG_ERROR("Trying to set the projectile velocity of an entity that has no projectile emitter component");
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

		// Create all the bindings between C++ and Lua functions
		luaState.set_function("get_position", GetEntityPosition);
		luaState.set_function("get_velocity", GetEntityVelocity);
		luaState.set_function("set_position", SetEntityPosition);
		luaState.set_function("set_velocity", SetEntityVelocity);
		luaState.set_function("set_rotation", SetEntityRotation);
		luaState.set_function("set_projectile_velocity", SetProjectileVelocity);
		luaState.set_function("set_animation_frame", SetEntityAnimationFrame);
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