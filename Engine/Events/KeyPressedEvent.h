#pragma once

#include "EventBus/Event.h"

#include <SDL3/SDL.h>

class KeyPressedEvent : public Event
{
public:
	SDL_Scancode keyScancode;

	KeyPressedEvent(SDL_Scancode keyScancode) : keyScancode(keyScancode) {}
};