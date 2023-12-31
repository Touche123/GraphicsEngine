#pragma once

#include <glm/vec3.hpp>
#include <fstream>

struct StaticPointLight {
	StaticPointLight() = default;
	StaticPointLight(const glm::vec3& color, const glm::vec3& position) : Color(color),
		Position(position)
	{
	}

	glm::vec3 Color;
	glm::vec3 Position;
};