#pragma once

#include <glm/glm.hpp>

struct TransformComponent
{
	TransformComponent(glm::vec2 position = glm::vec2(0, 0), glm::vec2 scale = glm::vec2(1.0, 1.0), double rotation = 0.0, bool isFixed = false)
	: position(position), scale(scale), rotation(rotation), isFixed(isFixed) { }

	glm::vec2 position;
	glm::vec2 scale;
	double rotation;

	bool isFixed;
};