#pragma once

#include "ECS/ECS.h"
#include "Components/KeyboardControlledComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/RigidBodyComponent.h"
#include "EventBus/EventBus.h"
#include "Events/KeyPressedEvent.h"

class KeyboardControlSystem : public System
{
public:
	KeyboardControlSystem()
	{
		RequireComponent<KeyboardControlledComponent>();
		RequireComponent<SpriteComponent>();
		RequireComponent<RigidBodyComponent>();
	}

	void SubscribeToEvents(EventBus& eventBus)
	{
		eventBus.SubscribeToEvent(this, &KeyboardControlSystem::OnKeyPressed);
	}

	void OnKeyPressed(KeyPressedEvent& event)
	{
		ForEach<KeyboardControlledComponent, SpriteComponent, RigidBodyComponent>([&](Entity entity, 
		const KeyboardControlledComponent& keyboardControl,
		SpriteComponent& sprite,
		RigidBodyComponent& rigidbody)
		{
			switch (event.keyScancode)
			{
			case SDL_SCANCODE_UP:
				rigidbody.velocity = keyboardControl.upVelocity;
				sprite.srcRect.y = sprite.height * 0; // This is horrible and static. This should and must be implemented in more generic and better way
				break;

			case SDL_SCANCODE_RIGHT:
				rigidbody.velocity = keyboardControl.rightVelocity;
				sprite.srcRect.y = sprite.height * 1; // This is horrible and static. This should and must be implemented in more generic and better way
				break;

			case SDL_SCANCODE_DOWN:
				rigidbody.velocity = keyboardControl.downVelocity;
				sprite.srcRect.y = sprite.height * 2; // This is horrible and static. This should and must be implemented in more generic and better way
				break;

			case SDL_SCANCODE_LEFT:
				rigidbody.velocity = keyboardControl.leftVelocity;
				sprite.srcRect.y = sprite.height * 3; // This is horrible and static. This should and must be implemented in more generic and better way
				break;

			default:
				break;
			}
		});
	}
};