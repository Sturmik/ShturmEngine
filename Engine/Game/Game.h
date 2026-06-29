#pragma once

#include <SDL3/SDL.h>

#include "ECS/ECS.h"
#include "EventBus/EventBus.h"
#include "AssetStore/AssetStore.h"

#include <glm/glm.hpp>

#include <sol/sol.hpp>

const int FPS = 60;
const int MILLISECONDS_PER_FRAME = 1000 / FPS;

class Game
{
public:
	Game();
	~Game();
	void Initialize();
	void Setup();
	void ProcessInput(SDL_Event& event);
	void Update();
	void Render();
	void Destroy();

	bool IsGameRunning();

	glm::vec2 GetWindowSize();

	glm::vec2 GetMapSize();

private:
	bool _isRunning;
	bool _isDebug;

	SDL_Window* _window;
	SDL_Renderer* _renderer;
	SDL_FRect _camera;

	sol::state _luaState;

	float _mapWidth;
	float _mapHeight;

	Registry _registry;
	EventBus _eventBus;

	SDL_AudioDeviceID _audioDevice;
};