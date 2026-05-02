#pragma once

#include <string>
#include <SDL3/SDL.h>

#include "AssetStore/AssetStore.h"

struct SpriteComponent
{
	SpriteComponent(std::string assetId = "", float width = 0.0, float height = 0.0, unsigned int zIndex = 0, float srcRectX = 0.0, float srcRectY = 0.0)
		: assetId(assetId), width(width), height(height), zIndex(zIndex)
	{
		this->srcRect = { srcRectX, srcRectY, width, height };
	}

	SpriteComponent(const AssetStore& assetStore, std::string assetId, unsigned int zIndex = 0, float srcRectX = 0.0, float srcRectY = 0.0)
		: assetId(assetId), zIndex(zIndex)
		{
			SDL_GetTextureSize(assetStore.GetTexture(assetId), &width, &height);

			this->srcRect = { srcRectX, srcRectY, width, height };
		}

	std::string assetId;
	float width;
	float height;
	SDL_FRect srcRect;

	unsigned int zIndex;
};