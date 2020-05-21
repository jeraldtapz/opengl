#pragma once
#include <glm/vec2.hpp>

struct kernel3
{
public:
	kernel3(const float kernel[9], const float offset);
	float kernel[9]{};
	glm::vec2 offset[9]
	{
		glm::vec2(-1, 1), // top-left
		glm::vec2(0.0f, 1), // top-center
		glm::vec2(1, 1), // top-right
		glm::vec2(-1, 0.0f),   // center-left
		glm::vec2(0.0f, 0.0f),   // center-center
		glm::vec2(1, 0.0f),   // center-right
		glm::vec2(-1, -1), // bottom-left
		glm::vec2(0.0f, -1), // bottom-center
		glm::vec2(1, -1)  // bottom-right    
	};
};
