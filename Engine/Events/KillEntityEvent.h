#pragma once

#include "EventBus/Event.h"
#include <string>

class KillEntityEvent : public Event
{
public:
    int entityId;

    KillEntityEvent(int entityId)
        : entityId(entityId) {}
};