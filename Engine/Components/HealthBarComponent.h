#pragma once

#include <string>
#include <glm/glm.hpp>
#include <SDL3/SDL.h>

struct HealthBarComponent
{
	std::string fontAssetId;
	glm::vec2 textOffset;

	glm::vec2 healthBarSize;
	glm::vec2 healthBarOffset;

	HealthBarComponent(std::string assetId = "", glm::vec2 textOffset = glm::vec2(0), glm::vec2 healthBarSize = glm::vec2(0), glm::vec2 healthBarOffset = glm::vec2(0))
		: fontAssetId(assetId), textOffset(textOffset), healthBarSize(healthBarSize), healthBarOffset(healthBarOffset) {}
};