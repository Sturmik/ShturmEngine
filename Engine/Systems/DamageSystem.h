#pragma once

#include "ECS/ECS.h"
#include "Components/BoxColliderComponent.h"
#include "EventBus/EventBus.h"
#include "Events/CollisionEvent.h"

class DamageSystem : public System
{
public:
	DamageSystem()
	{
		RequireComponent<BoxColliderComponent>();
	}

	void SubscribeToEvents(EventBus& eventBus)
	{
		eventBus.SubscribeToEvent(this, &DamageSystem::OnCollision);
	}

	void OnCollision(CollisionEvent& event)
	{
		LOG_INFO("Damage system received an event collision between entities %d and %d", event.a.GetId(), event.b.GetId());
		event.a.Kill();
		event.b.Kill();
	}

	void Update()
	{
		// ...
	}
};