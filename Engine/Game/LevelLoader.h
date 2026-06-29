#pragma once

#include "ECS/ECS.h"
#include <memory>
#include <SDL3/SDL.h>

#include <sol/sol.hpp>

class LevelLoader
{
public:
	LevelLoader();
	~LevelLoader();

	void LoadLevel(sol::state& luaState, Registry& registry, SDL_Window* window, SDL_Renderer* renderer, int levelNumber, float& outMapWidth, float& outMapHeight);
};