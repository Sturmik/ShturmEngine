#pragma once

#include "ECS/ECS.h"
#include "EventBus/EventBus.h"
#include "Events/KeyPressedEvent.h"

class KeyboardControlSystem : public System
{
public:
	KeyboardControlSystem()
	{
		// ...
	}

	void SubscribeToEvents(EventBus& eventBus)
	{
		eventBus.SubscribeToEvent(this, &KeyboardControlSystem::OnKeyPressed);
	}

	void OnKeyPressed(KeyPressedEvent& event)
	{
		LOG_INFO("Received key pressed event: %s", SDL_GetScancodeName(event.keyScancode));
	}
};