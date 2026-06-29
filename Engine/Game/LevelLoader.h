#pragma once

#include "ECS/ECS.h"
#include <memory>
#include <SDL3/SDL.h>

class LevelLoader
{
public:
	LevelLoader();
	~LevelLoader();

	void LoadLevel(Registry& registry, SDL_Window* window, SDL_Renderer* renderer, int level, float& outMapWidth, float& outMapHeight);
};