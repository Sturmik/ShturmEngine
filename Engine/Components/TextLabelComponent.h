#pragma once

#include <string>
#include <glm/glm.hpp>
#include <SDL3/SDL.h>

struct TextLabelComponent
{
	std::string text;
	std::string assetId;
	SDL_Color color;

	bool isFixed;

	TextLabelComponent(std::string text = "", std::string assetId = "", SDL_Color color = {0,0,0}, bool isFixed = false)
	: text(text), assetId(assetId), color(color), isFixed(isFixed) { }
};