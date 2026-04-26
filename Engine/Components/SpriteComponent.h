#pragma once

struct SpriteComponent
{
	SpriteComponent(float width = 0, float height = 0)
		: width(width), height(height) {
	}

	float width;
	float height;
};