#pragma once

struct SpriteComponent
{
	SpriteComponent(std::string assetId = "", float width = 0, float height = 0)
		: assetId(assetId), width(width), height(height) {
	}

	std::string assetId;
	float width;
	float height;
};