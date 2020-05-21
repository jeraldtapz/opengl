// jerald   Jerald Tapalla
// OpenGL  OpenGL  vertex.h
// 30 04 2019

#pragma once
#include <glm/vec2.hpp>

#include "rendering/color.h"
#include <glm/vec3.hpp>

struct vertex
{
public:
	glm::vec3 position{};
	glm::vec3 normal{};
	glm::vec2 tex_coord{};
	glm::vec3 tangent{};
	glm::vec3 bitangent{};


	vertex();
	vertex(float x, float y, float z);
	/*explicit vertex(const glm::vec3 position, const glm::vec3 normal, const glm::vec2 tex_coord, const glm::vec3 tangent, const glm::vec3 bitangent);*/
};
