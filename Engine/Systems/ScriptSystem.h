#pragma once

#include "ECS/ECS.h"
#include "Components/ScriptComponent.h"

class ScriptSystem : public System
{
public:
	ScriptSystem()
	{
		RequireComponent<ScriptComponent>();
	}

	void Update()
	{
		// Loop all entities that have a script component and invoke their Lua function
		ForEach<ScriptComponent>([&](Entity entity, const ScriptComponent& script)
		{
			// Invoke sol::function
			script.function();
		});
	}
};