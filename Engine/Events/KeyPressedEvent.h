#pragma once

#include "EventBus/EventBus.h"
#include "EventBus/Event.h"

#include <SDL3/SDL.h>

class KeyPressedEvent : public Event
{
public:
	SDL_Scancode keyScancode;
	EventBus* eventBusPtr;

	KeyPressedEvent(SDL_Scancode keyScancode, EventBus* eventBusPtr = nullptr) : keyScancode(keyScancode), eventBusPtr(eventBusPtr) {}
};