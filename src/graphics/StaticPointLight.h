#pragma once

#include <glm/vec3.hpp>
#include <fstream>

struct StaticPointLight {
	StaticPointLight() = default;
	StaticPointLight(const glm::vec3& color, const glm::vec3& position, const glm::vec3 rotation) : Color(color),
		Position(position), Rotation(rotation)
	{
	}

	glm::vec3 Color;
	glm::vec3 Rotation;
	glm::vec3 Position;
};