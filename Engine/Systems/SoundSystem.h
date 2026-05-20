#pragma once

#include "ECS/ECS.h"
#include "Components/SoundComponent.h"
#include "AssetStore/AssetStore.h"
#include "EventBus/EventBus.h"
#include "Events/PlaySoundEvent.h"
#include "Events/KillEntityEvent.h"

#include <SDL3/SDL.h>
#include <vector>
#include <unordered_map>

// Playing sound internal structure
struct PlayingSound
{
    SDL_AudioStream* stream = nullptr;
    const SoundData* soundData = nullptr;
    bool loop = false;
    int ownerEntityId = -1;
};

class SoundSystem : public System
{
public:
    SoundSystem()
    {
        RequireComponent<SoundComponent>();
    }

    ~SoundSystem()
    {
        Cleanup();
    }

    void Initialize(SDL_AudioDeviceID device)
    {
        _device = device;
    }

    void SubscribeToEvents(EventBus& eventBus)
    {
        eventBus.SubscribeToEvent(this, &SoundSystem::OnPlaySound);
        eventBus.SubscribeToEvent(this, &SoundSystem::OnKillEntity);
    }

    void Update()
    {
        for (std::shared_ptr<Archetype>& archetype : AccessArchetypes())
        {
            std::vector<Entity>& entities = archetype->entities;

            // Handle entity-attached sounds (e.g. looping engine sound)
            for (Entity& entity : entities)
            {
                SoundComponent& sound = entity.GetComponent<SoundComponent>();
                if (!sound.isInitialized && !sound.assetId.empty())
                {
                    PlaySound(sound.assetId, sound.loop, sound.volume, entity.GetId());
                    // one-shot trigger
                    sound.isInitialized = true;
                }
            }
        }

        // Clean up finished non-looping sounds or refill looped ones
        UpdateSounds();
    }

    void PlaySound(const std::string& assetId, bool loop = false, float volume = 1.0f, int ownerEntityId = -1)
    {
        SoundData* data = AssetStore::Get().GetSound(assetId);
        if (!data || !_device)
        {
            LOG_WARNING("Sound not found or audio not initialized: %s", assetId.c_str());
            return;
        }

        SDL_AudioStream* stream = SDL_CreateAudioStream(&data->spec, &data->spec);
        if (!stream)
        {
            LOG_ERROR("Failed to create audio stream for %s: %s", assetId.c_str(), SDL_GetError());
            return;
        }

        SDL_SetAudioStreamGain(stream, std::clamp(volume, 0.0f, 1.0f));

        SDL_PutAudioStreamData(stream, data->buffer, data->length);

        PlayingSound playingSound;
        playingSound.stream = stream;
        playingSound.soundData = data;
        playingSound.loop = loop;
        playingSound.ownerEntityId = ownerEntityId;

        if (!loop)
        {
            // Signal end of data for one-shot sounds
            SDL_FlushAudioStream(stream);
        }

        SDL_BindAudioStream(_device, stream);

        _playingSounds.push_back(playingSound);
        LOG_INFO("Playing sound: %s (loop: %d)", assetId.c_str(), loop);
    }

private:
    SDL_AudioDeviceID _device = 0;
    std::vector<PlayingSound> _playingSounds;

    void OnPlaySound(PlaySoundEvent& event)
    {
        PlaySound(event.assetId, event.loop, event.volume);
    }

    void OnKillEntity(KillEntityEvent& event)
    {
        // RemoveSwapLast all sounds belonging to the killed entity
        auto removeBegin = std::remove_if(_playingSounds.begin(), _playingSounds.end(),
            [&](const PlayingSound& ps)
            {
                if (ps.ownerEntityId == event.entityId)
                {
                    if (ps.stream)
                    {
                        SDL_DestroyAudioStream(ps.stream);
                    }
                    // Mark for removal
                    return true;
                }
                return false;
            });

        // Actually erase the marked elements
        _playingSounds.erase(removeBegin, _playingSounds.end());
    }

    void UpdateSounds()
    {
        for (auto it = _playingSounds.begin(); it != _playingSounds.end(); )
        {
            // RemoveSwapLast 
            if (SDL_GetAudioStreamAvailable(it->stream) == 0 && !it->loop)
            {
                SDL_DestroyAudioStream(it->stream);
                it = _playingSounds.erase(it);
            }
            else
            {
                // Refill looping sounds when they run out
                if (SDL_GetAudioStreamAvailable(it->stream) == 0 && it->loop)
                {
                    SDL_PutAudioStreamData(it->stream, it->soundData->buffer, it->soundData->length);
                }
                ++it;
            }
        }
    }

    void Cleanup()
    {
        for (auto& playingSound : _playingSounds)
        {
            if (playingSound.stream)
            {
                SDL_DestroyAudioStream(playingSound.stream);
            }
        }
        _playingSounds.clear();

        // Upper class owns it.
        _device = 0;
    }
};