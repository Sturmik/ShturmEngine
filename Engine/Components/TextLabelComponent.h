#pragma once

#include <string>
#include <glm/glm.hpp>
#include <SDL3/SDL.h>

struct TextLabelComponent
{
	std::string text;
	std::string assetId;
	SDL_Color color;

	TextLabelComponent(std::string text = "", std::string assetId = "", SDL_Color color = {0,0,0})
	: text(text), assetId(assetId), color(color) { }
};