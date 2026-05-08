#pragma once

#include "Logger/LoggerMacro.h"

#include "EventBus/Event.h"
#include <map>
#include <typeindex>

class IEventCallback
{
public:
	virtual ~IEventCallback() = default;

	void Execute(Event& e) 
	{
		Call(e);
	}

protected:
	virtual void Call(Event& e) = 0;
};

template<typename TOwner, typename TEvent>
class EventCallback : public IEventCallback
{
private:
	typedef void (TOwner::*CallbackFunction)(TEvent&);

public:
	EventCallback(TOwner* ownerInstance, CallbackFunction callbackFunction)
	 : _ownerInstance(ownerInstance), _callbackFunction(callbackFunction) {}

	virtual ~EventCallback() override = default;

protected:

	TOwner* _ownerInstance;
	CallbackFunction _callbackFunction;

	virtual void Call(Event& e) override
	{
		std::invoke(_callbackFunction, _ownerInstance, static_cast<TEvent&>(e));
	}
};

typedef std::vector<std::unique_ptr<IEventCallback>> HandlersList;

class EventBus
{
public:
	EventBus()
	{
		LOG_INFO("EventBus constructor called!");
	}

	~EventBus()
	{
		LOG_INFO("EventBus destructor called");
	}

	// Clears subscriber list
	void Reset()
	{
		_subscribers.clear();
	}
	
	// Subscribe to an event type <T>
	// A listener subscribes to an event
	// Example: eventBus->SubscribeToEvent<CollisionEvent>(this, &Game::onCollision);
	template<typename TOwner, typename TEvent>
	void SubscribeToEvent(TOwner* ownerInstance, void (TOwner::*callbackFunction)(TEvent&))
	{
		if (!_subscribers[typeid(TEvent)].get())
		{
			_subscribers[typeid(TEvent)] = std::make_unique<HandlersList>();
		}
		std::unique_ptr<IEventCallback> subscriber = std::make_unique<EventCallback<TOwner, TEvent>>(ownerInstance, callbackFunction);
		_subscribers[typeid(TEvent)]->emplace_back(std::move(subscriber));
	}

	// Emit an event of type <T>
	// As soon as something emits an
	// event we go ahead and execute all the listener callback functions
	// Example: eventBus->EmitEvent<CollisionEvent>(player, enemy);
	template<typename TEvent, typename ...TArgs>
	void EmitEvent(TArgs && ...args)
	{
		HandlersList* handlers = _subscribers[typeid(TEvent)].get();
		if (handlers)
		{
			for (auto it = handlers->begin(); it != handlers->end(); ++it)
			{
				IEventCallback* handler = it->get();
				TEvent event(std::forward<TArgs>(args)...);
				handler->Execute(event);
			}
		}
	}

private:
	std::map<std::type_index, std::unique_ptr<HandlersList>> _subscribers;
};