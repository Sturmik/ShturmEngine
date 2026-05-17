#pragma once

#include "EventBus/Event.h"
#include <string>

class PlaySoundEvent : public Event
{
public:
    std::string assetId;
    bool loop;
    float volume;

    PlaySoundEvent(const std::string& assetId, bool loop = false, float volume = 1.0f)
        : assetId(assetId), loop(loop), volume(volume) {}
};