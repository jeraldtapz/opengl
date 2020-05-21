#pragma once
#include <glm/gtc/matrix_transform.hpp>

struct mvp
{
	glm::mat4 model_matrix;
	glm::mat4 view;
	glm::mat4 projection;
};
